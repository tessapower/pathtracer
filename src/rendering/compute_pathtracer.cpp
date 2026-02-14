#include "stdafx.h"

#include "core/dx12_info_queue.h"
#include "rendering/compute_pathtracer.h"
#include "scene/camera.h"
#include "utils/d3dx12.h"
#include "utils/exception_macros.h"

#include <d3d12.h>
#include <dxgiformat.h>

#include <filesystem>
#include <fstream>
#include <sstream>

namespace pathtracer
{
ComputePathtracer::ComputePathtracer(ID3D12Device* device,
                                     DX12InfoQueue* infoQueue, UINT width,
                                     UINT height)
    : m_device(device), m_infoQueue(infoQueue), m_width(width), m_height(height)
{
    LoadComputeShader();
    CreateRootSignature();
    CreatePipelineState();
    CreateOutputTexture();
    CreateDescriptorHeap();
}

auto ComputePathtracer::Render(ID3D12GraphicsCommandList* commandList,
                               ID3D12Resource* renderTarget,
                               D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle,
                               const Camera& camera, const UINT frameIdx)
    -> void
{
    // Explicitly mark unused parameter to avoid warnings
    (void)rtvHandle;

    // Set the compute pipeline state and root signature
    // Tells GPU which shader to run and what resources to expect
    commandList->SetPipelineState(m_pipelineState.Get());
    commandList->SetComputeRootSignature(m_rootSignature.Get());

    // Bind UAV descriptor heap
    // Makes the descriptor heap (containing UAV descriptor) visible to shaders
    // n.b. you can bind multiple heaps, but we only have one
    ID3D12DescriptorHeap* heaps[] = {m_descriptorHeap.Get()};
    commandList->SetDescriptorHeaps(_countof(heaps), heaps);

    // Bind UAV root parameter
    // Binds the UAV descriptor to root parameter index 0, which matches the
    // root signature defined
    commandList->SetComputeRootDescriptorTable(
        0, // Root parameter index
        // Shaders access descriptors via GPU handles instead of CPU handles
        m_descriptorHeap->GetGPUDescriptorHandleForHeapStart());

    // Dispatch computer shader (runs on GPU)

    /// NOTE TO SELF:
    /// The compute shader will execute in parallel across many threads on the
    /// GPU. The Dispatch() call specifies how many groups of threads to
    /// launch. If the window is 1920 x 1080, and we have thread group size of
    /// 8x8 (from shader [numthreads(8, 8, 1)]), then we need:
    ///
    /// X: (1920 + 7) / 8 = 240 groups (sanity check: 240 * 8 = 1920)
    /// Y: (1080 + 7) / 8 = 135 groups (sanity check: 135 * 8 = 1080)
    /// Z: 1 group (we only need one layer of threads for 2D image)
    ///
    /// This gives us a total of 240 * 135 * 64 = 2,073,600 threads, which is
    /// enough to cover all 2,073,600 pixels in the output image.
    ///
    /// Why +7? Ceiling division ensures we cover all pixels, e.g.
    /// 1921 pixels, 8 pixels per group:
    /// 1921 / 8       = 240 groups -> misses 1 pixel
    /// (1921 + 7) / 8 = 241 groups

    UINT groupsX = (m_width + 7) / 8; // Ceiling division
    UINT groupsY = (m_height + 7) / 8;
    commandList->Dispatch(groupsX, groupsY, 1);

    // Wait for compute to finish (barrier)
    // Ensures all UAV writes from compute shader are complete before reading.
    // GPUs execute out-of-order, so this syncs compute writes with copy op.
    CD3DX12_RESOURCE_BARRIER uavBarrier =
        CD3DX12_RESOURCE_BARRIER::UAV(m_outputTexture.Get());
    commandList->ResourceBarrier(1, &uavBarrier);

    // Transition output texture: UAV to COPY_SOURCE
    CD3DX12_RESOURCE_BARRIER toCopySource =
        CD3DX12_RESOURCE_BARRIER::Transition(
            m_outputTexture.Get(), D3D12_RESOURCE_STATE_UNORDERED_ACCESS,
            D3D12_RESOURCE_STATE_COPY_SOURCE);
    commandList->ResourceBarrier(1, &toCopySource);

    // Transition render target: RENDER_TARGET to COPY_DEST
    CD3DX12_RESOURCE_BARRIER renderTargetToCopyDest =
        CD3DX12_RESOURCE_BARRIER::Transition(
            renderTarget, D3D12_RESOURCE_STATE_RENDER_TARGET,
            D3D12_RESOURCE_STATE_COPY_DEST);
    commandList->ResourceBarrier(1, &renderTargetToCopyDest);

    // Copy output texture to render target
    commandList->CopyResource(renderTarget, m_outputTexture.Get());

    // Transition render target back: COPY_DEST to RENDER_TARGET
    CD3DX12_RESOURCE_BARRIER renderTargetToRenderTarget =
        CD3DX12_RESOURCE_BARRIER::Transition(
            renderTarget, D3D12_RESOURCE_STATE_COPY_DEST,
            D3D12_RESOURCE_STATE_RENDER_TARGET);
    commandList->ResourceBarrier(1, &renderTargetToRenderTarget);

    // Transition output texture back to UNORDERED_ACCESS for next frame
    CD3DX12_RESOURCE_BARRIER toUAV = CD3DX12_RESOURCE_BARRIER::Transition(
        m_outputTexture.Get(), D3D12_RESOURCE_STATE_COPY_SOURCE,
        D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
    commandList->ResourceBarrier(1, &toUAV);
}

auto ComputePathtracer::Resize(const UINT width, const UINT height) -> void
{
    m_width = width;
    m_height = height;
    // Recreate output texture and descriptor heap to match new size
    CreateOutputTexture();
    CreateDescriptorHeap();
}

auto ComputePathtracer::LoadComputeShader() -> void
{
    std::filesystem::path shaderPath = m_COMPILED_SHADER_FILE;

    // Check if shader file exists
    if (!std::filesystem::exists(shaderPath))
    {
        std::ostringstream oss;
        oss << "Shader file not found: " << shaderPath << "\n"
            << "Working directory: " << std::filesystem::current_path() << "\n"
            << "Expected location : " << std::filesystem::absolute(shaderPath);

        throw std::runtime_error(oss.str());
    }

    // Open in binary mode, start at end (lets us get file size with tellg())
    std::ifstream shaderFile(shaderPath, std::ios::binary | std::ios::ate);
    if (!shaderFile.is_open())
    {
        throw std::runtime_error("Failed to open shader file: " +
                                 shaderPath.string());
    }

    // Get size and read
    size_t fileSize = shaderFile.tellg();
    shaderFile.seekg(0, std::ios::beg);

    m_shaderBytecode.resize(fileSize);
    if (!shaderFile.read(reinterpret_cast<char*>(m_shaderBytecode.data()),
                         fileSize))
    {
        throw std::runtime_error("Failed to read shader file: " +
                                 shaderPath.string());
    }

    // Validate we got the data
    if (m_shaderBytecode.empty())
    {
        throw std::runtime_error("Shader file is empty: " +
                                 shaderPath.string());
    }
}

auto ComputePathtracer::CreateRootSignature() -> void
{
    // v1.1 descriptor ranges
    CD3DX12_DESCRIPTOR_RANGE1 ranges[1];
    ranges[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 0, 0,
                   D3D12_DESCRIPTOR_RANGE_FLAG_DATA_VOLATILE); // U0

    // Root parameter with v1.1
    CD3DX12_ROOT_PARAMETER1 rootParams[1];
    rootParams[0].InitAsDescriptorTable(1, &ranges[0]);

    // Versioned root signature descriptor (1.1)
    CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC rootSigDesc;
    rootSigDesc.Init_1_1(_countof(rootParams), rootParams, 0, nullptr,
                         D3D12_ROOT_SIGNATURE_FLAG_NONE);

    // Serialize and create root signature
    Microsoft::WRL::ComPtr<ID3DBlob> signature, error;
#ifdef _DEBUG
    if (m_infoQueue)
    {
        DX12_CHECK_MSG(D3DX12SerializeVersionedRootSignature(
                           &rootSigDesc, D3D_ROOT_SIGNATURE_VERSION_1_1,
                           &signature, &error),
                       *m_infoQueue);
    }
    else
#endif // _DEBUG
    {
        DX12_CHECK(D3DX12SerializeVersionedRootSignature(
            &rootSigDesc, D3D_ROOT_SIGNATURE_VERSION_1_1, &signature, &error));
    }

#ifdef _DEBUG
    if (m_infoQueue)
    {
        DX12_CHECK_MSG(
            m_device->CreateRootSignature(0, signature->GetBufferPointer(),
                                          signature->GetBufferSize(),
                                          IID_PPV_ARGS(&m_rootSignature)),
            *m_infoQueue);
    }
    else
#endif // _DEBUG
    {
        DX12_CHECK(m_device->CreateRootSignature(
            0, signature->GetBufferPointer(), signature->GetBufferSize(),
            IID_PPV_ARGS(&m_rootSignature)));
    }
}

auto ComputePathtracer::CreatePipelineState() -> void
{
    // Create compute pipeline state descriptor
    D3D12_COMPUTE_PIPELINE_STATE_DESC psoDesc = {};

    // Link to root signature (what resources shader can access)
    psoDesc.pRootSignature = m_rootSignature.Get();

    // Provide compiled shader bytecode
    psoDesc.CS.pShaderBytecode = m_shaderBytecode.data();
    psoDesc.CS.BytecodeLength = m_shaderBytecode.size();

    // Single GPU
    psoDesc.NodeMask = 0;

    // No cached PSO
    psoDesc.CachedPSO.pCachedBlob = nullptr;
    psoDesc.CachedPSO.CachedBlobSizeInBytes = 0;

    // Standard flags
    psoDesc.Flags = D3D12_PIPELINE_STATE_FLAG_NONE;

// Create the PSO
#ifdef _DEBUG
    if (m_infoQueue)
    {
        DX12_CHECK_MSG(m_device->CreateComputePipelineState(
                           &psoDesc, IID_PPV_ARGS(&m_pipelineState)),
                       *m_infoQueue);
    }
    else
#endif
    {
        DX12_CHECK(m_device->CreateComputePipelineState(
            &psoDesc, IID_PPV_ARGS(&m_pipelineState)));
    }
}

auto ComputePathtracer::CreateOutputTexture() -> void
{
    // Use helper to build resource description
    CD3DX12_RESOURCE_DESC texDesc = CD3DX12_RESOURCE_DESC::Tex2D(
        DXGI_FORMAT_R16G16B16A16_FLOAT,            // Format, match swapchain
        m_width,                                   // Width
        m_height,                                  // Height
        1,                                         // Array size
        1,                                         // Mip levels
        1,                                         // Sample count
        0,                                         // Sample quality
        D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS // UAV flag
    );

    // Default heap type for GPU read/write access. The GPU will manage
    // memory location and paging.
    CD3DX12_HEAP_PROPERTIES heapProps(D3D12_HEAP_TYPE_DEFAULT);

// Create the texture resource
#ifdef _DEBUG
    if (m_infoQueue)
    {
        DX12_CHECK_MSG(
            m_device->CreateCommittedResource(
                &heapProps,                            // GPU memory
                D3D12_HEAP_FLAG_NONE,                  // No special flags
                &texDesc,                              // Texture description
                D3D12_RESOURCE_STATE_UNORDERED_ACCESS, //  Initial state
                                                       // (shader writeable)
                nullptr, // No clear value (not a render target)
                IID_PPV_ARGS(&m_outputTexture)), // Output COM ptr
            *m_infoQueue);
    }
    else
#endif // _DEBUG
    {
        DX12_CHECK(m_device->CreateCommittedResource(
            &heapProps,                            // GPU memory
            D3D12_HEAP_FLAG_NONE,                  // No special flags
            &texDesc,                              // Texture description
            D3D12_RESOURCE_STATE_UNORDERED_ACCESS, //  Initial state
                                                   // (shader writeable)
            nullptr, // No clear value (not a render target)
            IID_PPV_ARGS(&m_outputTexture)) // Output COM ptr
        );
    }

    m_outputTexture->SetName(L"ComputePathTracer Output Texture");
}

auto ComputePathtracer::CreateDescriptorHeap() -> void
{
    // Create descriptor heap for UAV
    D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {};
    heapDesc.NumDescriptors = 1; // Only need one UAV descriptor
    heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
    heapDesc.Flags =
        D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE; // Visible to shader
    heapDesc.NodeMask = 0;                         // Single GPU

    // Create the descriptor heap
#ifdef _DEBUG
    if (m_infoQueue)
    {
        DX12_CHECK_MSG(m_device->CreateDescriptorHeap(
                           &heapDesc, IID_PPV_ARGS(&m_descriptorHeap)),
                       *m_infoQueue);
    }
    else
#endif // _DEBUG
    {
        DX12_CHECK(m_device->CreateDescriptorHeap(
            &heapDesc, IID_PPV_ARGS(&m_descriptorHeap)));
    }

    // Get descriptor size for late use when creating views
    m_descriptorSize = m_device->GetDescriptorHandleIncrementSize(
        D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

    // Create UAV descriptor
    D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
    uavDesc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
    uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D; // 2D texture
    uavDesc.Texture2D.MipSlice = 0;                        // First mip level
    uavDesc.Texture2D.PlaneSlice = 0;                      // Single plane

    CD3DX12_CPU_DESCRIPTOR_HANDLE cpuHandle(
        m_descriptorHeap->GetCPUDescriptorHandleForHeapStart());

    // Create the view
    m_device->CreateUnorderedAccessView(m_outputTexture.Get(),
                                        nullptr, // No counter resource
                                        &uavDesc, cpuHandle);
}

} // namespace pathtracer

#include "stdafx.h"

#include "core/dx12_info_queue.h"
#include "rendering/renderer.h"
#include "utils/d3dx12.h"
#include "utils/exception_macros.h"

#include <d3d12.h>
#include <dxgi1_6.h>

#include <memory>
#include <stdexcept>

namespace pathtracer
{
Renderer::Renderer(ID3D12Device* device, IDXGIFactory4* factory,
                   ID3D12CommandQueue* commandQueue, DX12InfoQueue* infoQueue,
                   HWND hwnd, UINT width, UINT height)
    : m_device(device), m_commandQueue(commandQueue), m_infoQueue(infoQueue)
{
    // Create RTV descriptor heap
    D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {};
    rtvHeapDesc.NumDescriptors = SwapChain::BufferCount;
    rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
    rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;

#ifdef _DEBUG
    if (m_infoQueue)
    {
        DX12_CHECK_MSG(m_device->CreateDescriptorHeap(&rtvHeapDesc,
                                                      IID_PPV_ARGS(&m_rtvHeap)),
                       *m_infoQueue);
    }
    else
#endif // _DEBUG
    {
        DX12_CHECK(m_device->CreateDescriptorHeap(&rtvHeapDesc,
                                                  IID_PPV_ARGS(&m_rtvHeap)));
    }

    m_rtvDescriptorSize = m_device->GetDescriptorHandleIncrementSize(
        D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

    // Create swapchain
    m_swapChain = std::make_unique<SwapChain>(device, factory, commandQueue,
                                              infoQueue, hwnd, width, height);

    // Create one command allocator per back buffer
    for (auto i = 0; i < SwapChain::BufferCount; ++i)
    {
#ifdef _DEBUG
        if (m_infoQueue)
        {
            DX12_CHECK_MSG(m_device->CreateCommandAllocator(
                               D3D12_COMMAND_LIST_TYPE_DIRECT,
                               IID_PPV_ARGS(&m_commandAllocators[i])),
                           *m_infoQueue);
        }
        else
#endif // _DEBUG
        {
            DX12_CHECK(m_device->CreateCommandAllocator(
                D3D12_COMMAND_LIST_TYPE_DIRECT,
                IID_PPV_ARGS(&m_commandAllocators[i])));
        }
    }

    // Create command list

#ifdef _DEBUG
    if (m_infoQueue)
    {
        DX12_CHECK_MSG(
            m_device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT,
                                        m_commandAllocators[0].Get(), nullptr,
                                        IID_PPV_ARGS(&m_commandList)),
            *m_infoQueue);
    }
    else
#endif // _DEBUG
    {
        DX12_CHECK(m_device->CreateCommandList(
            0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_commandAllocators[0].Get(),
            nullptr, IID_PPV_ARGS(&m_commandList)));
    }

    // Close command list
#ifdef _DEBUG
    if (m_infoQueue)
    {
        DX12_CHECK_MSG(m_commandList->Close(), *m_infoQueue);
    }
    else
#endif // _DEBUG
    {
        DX12_CHECK(m_commandList->Close());
    }

    // Create fence
#ifdef _DEBUG
    if (m_infoQueue)
    {
        DX12_CHECK_MSG(m_device->CreateFence(0, D3D12_FENCE_FLAG_NONE,
                                             IID_PPV_ARGS(&m_fence)),
                       *m_infoQueue);
    }
    else
#endif // _DEBUG
    {
        DX12_CHECK(m_device->CreateFence(0, D3D12_FENCE_FLAG_NONE,
                                         IID_PPV_ARGS(&m_fence)));
    }

    // Create fence event
    m_fenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
    if (!m_fenceEvent)
    {
        // TODO: replace with pathtracer error handling
        throw std::runtime_error("Failed to create fence event");
    }

    // Init fence values
    for (auto i = 0; i < SwapChain::BufferCount; ++i)
    {
        m_fenceValues[i] = 1;
    }

    CreateBackBufferRTVs();
}

auto Renderer::RenderFrame() -> void
{
    const UINT frameIdx = m_swapChain->GetCurrentBackBufferIndex();

    // Reset command allocator
#ifdef _DEBUG
    if (m_infoQueue)
    {
        DX12_CHECK_MSG(m_commandAllocators[frameIdx]->Reset(), *m_infoQueue);
    }
    else
#endif // _DEBUG
    {
        DX12_CHECK(m_commandAllocators[frameIdx]->Reset());
    }

    // Reset command list

#ifdef _DEBUG
    if (m_infoQueue)
    {
        DX12_CHECK_MSG(
            m_commandList->Reset(m_commandAllocators[frameIdx].Get(), nullptr),
            *m_infoQueue);
    }
    else
#endif // _DEBUG
    {
        DX12_CHECK(
            m_commandList->Reset(m_commandAllocators[frameIdx].Get(), nullptr));
    }

    // Get back buffer
    ID3D12Resource* backBuffer = m_swapChain->GetCurrentBackBuffer();

    // Transition to render target
    CD3DX12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(
        backBuffer, D3D12_RESOURCE_STATE_PRESENT,
        D3D12_RESOURCE_STATE_RENDER_TARGET);

    m_commandList->ResourceBarrier(1, &barrier);

    // Get RTV handle for this back buffer
    CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(
        m_rtvHeap->GetCPUDescriptorHandleForHeapStart(), frameIdx,
        m_rtvDescriptorSize);

    // Clear to background color
    m_commandList->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);

    // Transition back to present
    barrier = CD3DX12_RESOURCE_BARRIER::Transition(
        backBuffer, D3D12_RESOURCE_STATE_RENDER_TARGET,
        D3D12_RESOURCE_STATE_PRESENT);

    m_commandList->ResourceBarrier(1, &barrier);

    // Close command list
#ifdef _DEBUG
    if (m_infoQueue)
    {
        DX12_CHECK_MSG(m_commandList->Close(), *m_infoQueue);
    }
    else
#endif // _DEBUG
    {
        DX12_CHECK(m_commandList->Close());
    }

    // Execute command list
    ID3D12CommandList* commandLists[] = {m_commandList.Get()};
    m_commandQueue->ExecuteCommandLists(1, commandLists);

    // Present
    m_swapChain->Present(true);

    // Signal the fence
    const UINT64 currentFenceValue = m_fenceValues[frameIdx];

#ifdef _DEBUG
    if (m_infoQueue)
    {
        DX12_CHECK_MSG(m_commandQueue->Signal(m_fence.Get(), currentFenceValue),
                       *m_infoQueue);
    }
    else
#endif // _DEBUG
    {
        DX12_CHECK(m_commandQueue->Signal(m_fence.Get(), currentFenceValue));
    }

    // Wait for previous frame
    const UINT nextFrameIdx = m_swapChain->GetCurrentBackBufferIndex();
    if (m_fence->GetCompletedValue() < m_fenceValues[nextFrameIdx])
    {
        m_fence->SetEventOnCompletion(m_fenceValues[nextFrameIdx],
                                      m_fenceEvent);
        WaitForSingleObject(m_fenceEvent, INFINITE);
    }

    // Update fence value for next frame
    m_fenceValues[nextFrameIdx] = currentFenceValue + 1;
}

auto Renderer::OnResize(const UINT width, const UINT height) -> void
{
    // Need to idle first so we don't swap anything out while in use
    WaitForGpu();

    // Swapchain releases old buffers and gets new ones
    m_swapChain->Resize(width, height);

    // Recreate render target views for the new buffers
    CreateBackBufferRTVs();
}

auto Renderer::WaitForGpu() -> void
{
    // Signal and wait for GPU to finish
    const UINT64 fenceValue =
        m_fenceValues[m_swapChain->GetCurrentBackBufferIndex()];

#ifdef _DEBUG
    if (m_infoQueue)
    {
        DX12_CHECK_MSG(m_commandQueue->Signal(m_fence.Get(), fenceValue),
                       *m_infoQueue);
    }
    else
#endif // _DEBUG
    {
        DX12_CHECK(m_commandQueue->Signal(m_fence.Get(), fenceValue));
    }

#ifdef _DEBUG
    if (m_infoQueue)
    {
        DX12_CHECK_MSG(m_fence->SetEventOnCompletion(fenceValue, m_fenceEvent),
                       *m_infoQueue);
    }
    else
#endif // _DEBUG
    {
        DX12_CHECK(m_fence->SetEventOnCompletion(fenceValue, m_fenceEvent));
    }

    WaitForSingleObject(m_fenceEvent, INFINITE);

    // Increment fence value for next frame
    m_fenceValues[m_swapChain->GetCurrentBackBufferIndex()]++;
}

auto Renderer::CreateBackBufferRTVs() -> void
{
    // Wrapper for descriptor handles with helper methods
    CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(
        m_rtvHeap->GetCPUDescriptorHandleForHeapStart());

    const auto& backBuffers = m_swapChain->GetBackBuffers();

    for (auto i = 0; i < SwapChain::BufferCount; ++i)
    {
        m_device->CreateRenderTargetView(backBuffers[i].Get(), nullptr,
                                         rtvHandle);
        rtvHandle.Offset(1, m_rtvDescriptorSize);
    }
}

} // namespace pathtracer

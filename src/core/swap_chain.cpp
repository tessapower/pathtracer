#include "stdafx.h"

#include "core/swap_chain.h"
#include "utils/string_utils.h"

#include <iostream>

namespace pathtracer
{

SwapChain::SwapChain(ID3D12Device* device, IDXGIFactory4* factory,
                     ID3D12CommandQueue* commandQueue, HWND hwnd, UINT width,
                     UINT height)
    : m_device(device), m_width(width), m_height(height)
{
    // Create swap chain description
    DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
    swapChainDesc.Width = width;
    swapChainDesc.Height = height;
    // Use 16-bit float RGBA format for HDR back buffers
    swapChainDesc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
    swapChainDesc.Stereo = FALSE;
    swapChainDesc.SampleDesc.Count = 1; // No MSAA
    swapChainDesc.SampleDesc.Quality = 0;
    swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    swapChainDesc.BufferCount = BufferCount;
    swapChainDesc.Scaling = DXGI_SCALING_STRETCH; // Scale to window size
    // Discard previous frame after presenting (flip model)
    swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
    swapChainDesc.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;
    swapChainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING; // Allow tearing

    // Create swap chain
    Microsoft::WRL::ComPtr<IDXGISwapChain1> swapChain1;
    if (FAILED(factory->CreateSwapChainForHwnd(
        commandQueue, hwnd, &swapChainDesc,
        nullptr, // Fullscreen descriptor (nullptr = windowed)
        nullptr, // Restrict output (nullptr = default)
        &swapChain1)))
    {
        throw std::runtime_error("Failed to create swap chain");
    }

    // Upgrade to IDXGISwapChain4
    if (FAILED(swapChain1.As(&m_swapChain)))
    {
        throw std::runtime_error("Failed to upgrade swap chain to IDXGISwapChain4");
    }

    // Disable Alt+Enter fullscreen toggle (handle it manually if needed)
    if (FAILED(factory->MakeWindowAssociation(hwnd, DXGI_MWA_NO_ALT_ENTER)))
    {
        throw std::runtime_error("Failed to disable Alt+Enter fullscreen toggle");
    }

    // Create render target views for back buffers
    RetrieveBackBuffers();

    OutputDebugString(TEXT("Swapchain Created"));
}

SwapChain::~SwapChain()
{
    // Release render targets
    ReleaseRenderTargets();
    OutputDebugString(TEXT("Swapchain destroyed\n"));
}

void SwapChain::Present(bool vsync)
{
    // Present the frame
    UINT syncInterval = vsync ? 1 : 0;
    // Can't use tearing with vsync enabled
    UINT presentFlags = vsync ? 0 : DXGI_PRESENT_ALLOW_TEARING;
    m_swapChain->Present(syncInterval, presentFlags);
}

void SwapChain::Resize(int width, int height)
{
    ReleaseRenderTargets();

    // Resize with tearing support
    HRESULT hr = m_swapChain->ResizeBuffers(
        BufferCount, width, height, DXGI_FORMAT_R16G16B16A16_FLOAT,
        DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING // Maintain tearing support
    );

    if (FAILED(hr))
    {
        throw std::runtime_error("Failed to resize swap chain buffers");
    }

    // Update dimensions
    m_width = width;
    m_height = height;

    RetrieveBackBuffers();
}

UINT SwapChain::GetCurrentBackBufferIndex() const
{
    // Get current back buffer index
    return m_swapChain->GetCurrentBackBufferIndex();
}

ID3D12Resource* SwapChain::GetCurrentBackBuffer() const
{
    // Return current back buffer resource
    UINT index = GetCurrentBackBufferIndex();
    return m_backBuffers[index].Get();
}

void SwapChain::RetrieveBackBuffers()
{
    // Get back buffers from swap chain
    m_backBuffers.resize(BufferCount);
    for (UINT i = 0; i < BufferCount; ++i)
    {
        if (FAILED(m_swapChain->GetBuffer(i, IID_PPV_ARGS(&m_backBuffers[i]))))
        {
            throw std::runtime_error("Failed to get back buffer from swap chain");
        }
    }
}

void SwapChain::ReleaseRenderTargets()
{
    // Release all back buffer references
    for (auto& buffer : m_backBuffers)
        buffer.Reset();
}

} // namespace pathtracer

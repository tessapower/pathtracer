#include "stdafx.h"

#include "core/swap_chain.h"
#include "utils/exception_macros.h"
#include "utils/string_utils.h"

#include <dxgi.h>
#include <dxgiformat.h>
#include <dxgi1_2.h>

#include <iostream>

namespace pathtracer
{

SwapChain::SwapChain(ID3D12Device* device, IDXGIFactory4* factory,
                     ID3D12CommandQueue* commandQueue,
                     DX12InfoQueue* infoQueue, HWND hwnd, UINT width,
                     UINT height)
    : m_device(device), m_infoQueue(infoQueue), m_width(width), m_height(height)
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
    swapChainDesc.BufferCount = BUFFER_COUNT;
    swapChainDesc.Scaling = DXGI_SCALING_STRETCH; // Scale to window size
    // Discard previous frame after presenting (flip model)
    swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
    swapChainDesc.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;
    swapChainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING; // Allow tearing

    // Create swap chain
    Microsoft::WRL::ComPtr<IDXGISwapChain1> swapChain1;

#ifdef _DEBUG
    if (m_infoQueue)
    {
        DX12_CHECK_MSG(
            factory->CreateSwapChainForHwnd(
                commandQueue, hwnd, &swapChainDesc,
                nullptr, // Fullscreen descriptor (nullptr = windowed)
                nullptr, // Restrict output (nullptr = default)
                &swapChain1),
            *m_infoQueue);
    }
    else
#endif // _DEBUG
    {
        DX12_CHECK(factory->CreateSwapChainForHwnd(
            commandQueue, hwnd, &swapChainDesc,
            nullptr, // Fullscreen descriptor (nullptr = windowed)
            nullptr, // Restrict output (nullptr = default)
            &swapChain1));
    }

    // Upgrade to IDXGISwapChain4
#ifdef _DEBUG
    if (m_infoQueue)
    {
        DX12_CHECK_MSG(swapChain1.As(&m_swapChain), *m_infoQueue);
    }
    else
#endif // _DEBUG
    {
        DX12_CHECK(swapChain1.As(&m_swapChain));
    }

    // Disable Alt+Enter fullscreen toggle (handle it manually if needed)
#ifdef _DEBUG
    if (m_infoQueue)
    {
        DX12_CHECK_MSG(
            factory->MakeWindowAssociation(hwnd, DXGI_MWA_NO_ALT_ENTER),
            *m_infoQueue);
    }
    else
#endif // _DEBUG
    {
        DX12_CHECK(factory->MakeWindowAssociation(hwnd, DXGI_MWA_NO_ALT_ENTER));
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

#ifdef _DEBUG
    if (m_infoQueue)
    {
        DX12_CHECK_MSG(m_swapChain->Present(syncInterval, presentFlags),
                       *m_infoQueue);
    }
    else
#endif // _DEBUG
    {
        DX12_CHECK(m_swapChain->Present(syncInterval, presentFlags));
    }
}

void SwapChain::Resize(int width, int height)
{
    ReleaseRenderTargets();

    // Resize with tearing support
#ifdef _DEBUG
    if (m_infoQueue)
    {
        DX12_CHECK_MSG(
            m_swapChain->ResizeBuffers(
                BUFFER_COUNT, width, height, DXGI_FORMAT_R16G16B16A16_FLOAT,
                DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING // Maintain tearing support
                ),
            *m_infoQueue);
    }
    else
#endif // _DEBUG
    {
        DX12_CHECK(m_swapChain->ResizeBuffers(
            BUFFER_COUNT, width, height, DXGI_FORMAT_R16G16B16A16_FLOAT,
            DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING // Maintain tearing support
            ));
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
    m_backBuffers.resize(BUFFER_COUNT);
    for (UINT i = 0; i < BUFFER_COUNT; ++i)
    {
#ifdef _DEBUG
        if (m_infoQueue)
        {
            DX12_CHECK_MSG(
                m_swapChain->GetBuffer(i, IID_PPV_ARGS(&m_backBuffers[i])),
                *m_infoQueue);
        }
        else
#endif // _DEBUG
        {
            DX12_CHECK(
                m_swapChain->GetBuffer(i, IID_PPV_ARGS(&m_backBuffers[i])));
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

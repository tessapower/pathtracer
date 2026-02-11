#pragma once

#include "core/dx12_info_queue.h"
#include "core/swap_chain.h"
#include "platform/windows_fwd.h"

#include <d3d12.h>
#include <dxgi1_6.h>

#include <memory>
#include <wrl.h>

namespace pathtracer
{

class Renderer
{
  public:
    /// <summary>
    /// Initializes the renderer with the given D3D12 device, command queue,
    /// info queue and swap chain.
    /// <param name="device">Pointer to the D3D12 device</param>
    /// <param name="factory">Pointer to the DXGI factory</param>
    /// <param name="commandQueue">Pointer to the D3D12 command queue</param>
    /// <param name="infoQueue">Pointer to the DX12 info queue</param>
    /// <param name="hwnd">Handle to the window</param>
    /// <param name="width">Width of the window</param>
    /// <param name="height">Height of the window</param>
    /// </summary>
    Renderer(ID3D12Device* device, IDXGIFactory4* factory,
             ID3D12CommandQueue* commandQueue, DX12InfoQueue* infoQueue,
             HWND hwnd, UINT width, UINT height);
    ~Renderer() = default;

    /// <summary>
    /// </summary>
    auto RenderFrame(/*const Camera& camera, const Scene& scene*/) -> void;

    /// <summary>
    /// </summary>
    auto OnResize(const UINT width, const UINT height) -> void;

    /// <summary>
    /// </summary>
    auto WaitForGpu() -> void;

    /// <summary>
    /// </summary>
    auto SetPathtracerType(/* PathtracerType type*/) -> void;

  private:
    const float clearColor[4] = {0.4f, 0.6f, 0.9f, 1.0f};

    auto CreateBackBufferRTVs() -> void;

    std::unique_ptr<SwapChain> m_swapChain;
    ID3D12Device* m_device;             // Non-owning pointer
    ID3D12CommandQueue* m_commandQueue; // Non-owning pointer
    DX12InfoQueue* m_infoQueue;

    // Descriptor heaps (RTV, CBV/SRV/UAV)
    Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_rtvHeap;
    UINT m_rtvDescriptorSize;
    Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_srvHeap;

    // Shared across all frames so only one exists!
    Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> m_commandList;
    Microsoft::WRL::ComPtr<ID3D12Fence> m_fence;
    HANDLE m_fenceEvent; // CPU notification

    // Sized by BufferCount so one per frame-in-flight!
    Microsoft::WRL::ComPtr<ID3D12CommandAllocator>
        m_commandAllocators[SwapChain::BufferCount];
    UINT64 m_fenceValues[SwapChain::BufferCount];
};
} // namespace pathtracer

#ifndef SWAP_CHAIN_H
#define SWAP_CHAIN_H

#include "core/dx12_info_queue.h"

#include <d3d12.h>
#include <dxgi1_6.h>

#include <vector>
#include <wrl.h>

namespace pathtracer
{
/// <summary>
/// DX12 Swap Chain wrapper:
/// <para></para>
/// Manages the swap chain and back buffers for presenting to the window.
/// <para></para>
/// <para>Key DX12 concepts:</para>
/// <para>- IDXGISwapChain4: Modern swap chain interface</para>
/// <para>- Flip model (DXGI_SWAP_EFFECT_FLIP_DISCARD)</para>
/// <para>- Back buffers (render targets)</para>
/// <para>- Present(): Display frame</para>
/// <para>- GetCurrentBackBufferIndex(): Which buffer to render to</para>
/// <para>- Synchronization with fences</para>
/// </summary>
class SwapChain
{
  public:
    static constexpr UINT BufferCount = 2; // Double buffering

    /// <summary>
    /// Constructor: Create swap chain and back buffers
    /// <param name="device">Pointer to the D3D12 device</param>
    /// <param name="factory">Pointer to the DXGI factory</param>
    /// <param name="commandQueue">Pointer to the command queue</param>
    /// <param name="infoQueue">Pointer to the D3D12 info queue</param>
    /// <param name="hwnd">Handle to the window</param>
    /// <param name="width">Width of the swap chain buffers</param>
    /// <param name="height">Height of the swap chain buffers</param>
    /// </summary>
    SwapChain(ID3D12Device* device, IDXGIFactory4* factory,
              ID3D12CommandQueue* commandQueue, DX12InfoQueue* infoQueue,
              HWND hwnd, UINT width, UINT height);
    ~SwapChain();

    // Disable copy/move constructors and assignment
    SwapChain(const SwapChain&) = delete;
    SwapChain& operator=(const SwapChain&) = delete;

    /// <summary>
    /// Present the current frame to the window
    /// <param name="vsync">Enable vertical sync (1, default) or not (0)</param>
    /// </summary>
    void Present(bool vsync = true);

    /// <summary>
    /// Resize swap chain buffers
    /// <para></para>
    /// Must wait for GPU to idle before calling
    /// </summary>
    void Resize(int width, int height);

    /// <summary>
    /// Get current back buffer index
    /// </summary>
    UINT GetCurrentBackBufferIndex() const;

    /// <summary>
    /// Get render target view for current back buffer
    /// </summary>
    ID3D12Resource* GetCurrentBackBuffer() const;

    /// <summary>
    /// Get all back buffer resources
    /// </summary>
    const std::vector<Microsoft::WRL::ComPtr<ID3D12Resource>>&
    GetBackBuffers() const
    {
        return m_backBuffers;
    }

  private:
    void RetrieveBackBuffers();
    void ReleaseRenderTargets();

  private:
    Microsoft::WRL::ComPtr<IDXGISwapChain4> m_swapChain;
    std::vector<Microsoft::WRL::ComPtr<ID3D12Resource>> m_backBuffers;
    ID3D12Device* m_device = nullptr; // non-owning pointer
    DX12InfoQueue* m_infoQueue = nullptr; // non-owning pointer
    UINT m_width = 0;
    UINT m_height = 0;
};

} // namespace pathtracer

#endif // SWAP_CHAIN_H

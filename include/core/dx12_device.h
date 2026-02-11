#ifndef DX12_DEVICE_H
#define DX12_DEVICE_H

#include "core/dx12_info_queue.h"

#include <d3d12.h>
#include <dxgi1_6.h>

#include <memory>
#include <wrl.h>

namespace pathtracer
{
/// <summary>
/// DX12 Device Management:
/// <para></para>
/// <para>- Device initialization with feature level validation</para>
/// <para>- Debug layer integration for development builds</para>
/// <para>- DXGI adapter enumeration and selection</para>
/// <para>- Command queue creation</para>
/// </summary>
class DX12Device
{
  public:
    DX12Device();
    ~DX12Device() = default;

    /// Getter methods for device, command queue, factory, and adapter

    auto GetDevice() const -> ID3D12Device*
    {
        return m_device.Get();
    }

    auto GetCommandQueue() const -> ID3D12CommandQueue*
    {
        return m_commandQueue.Get();
    }

    auto GetFactory() const -> IDXGIFactory6*
    {
        return m_factory.Get();
    }

    auto GetAdapter() const -> IDXGIAdapter1*
    {
        return m_adapter.Get();
    }

    auto GetInfoQueue() const -> DX12InfoQueue* {
        return m_infoQueue.get();
    }

  private:
    /// <summary>
    /// Loads the rendering pipeline.
    /// </summary>
    auto LoadPipeline() -> void;

    Microsoft::WRL::ComPtr<ID3D12Device> m_device;
    Microsoft::WRL::ComPtr<IDXGIFactory6> m_factory;
    Microsoft::WRL::ComPtr<IDXGIAdapter1> m_adapter;
    Microsoft::WRL::ComPtr<ID3D12CommandQueue> m_commandQueue;
    std::unique_ptr<DX12InfoQueue> m_infoQueue;
};

#endif // DX12_DEVICE_H

} // namespace pathtracer

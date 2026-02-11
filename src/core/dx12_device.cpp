#include "stdafx.h"

#include "core/dx12_device.h"
#include "utils/exception_macros.h"

// DX12 headers
#include <d3d12.h>
#include <dxgi1_6.h>

#include <stdexcept>

namespace pathtracer
{
DX12Device::DX12Device()
{
    LoadPipeline();
}

auto DX12Device::LoadPipeline() -> void
{
// Enable the debug layer
#ifdef _DEBUG
    {
        Microsoft::WRL::ComPtr<ID3D12Debug> debugController;

        /// NOTE TO SELF:
        // IID_PPV_ARGS = "Interface ID, Pointer to Pointer to Interface"
        // Helper macro that:
        // 1. Takes address of pointer to the interface (&debugController)
        // 2. Casts it to void** (D3D12GetDebugInterface expects a void**)
        // 3. Passes IID of the interface (e.g. __uuidof(ID3D12Debug))
        //
        // So it effectively does:
        // D3D12GetDebugInterface(__uuidof(ID3D12Debug),
        // (void**)&debugController)

        if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController))))
        {
            debugController->EnableDebugLayer();
        }
    }
#endif

    // Create Factory
    UINT createFactoryFlags = 0;
#ifdef _DEBUG
    createFactoryFlags |= DXGI_CREATE_FACTORY_DEBUG;
#endif
    DX12_CHECK(
        CreateDXGIFactory2(createFactoryFlags, IID_PPV_ARGS(&m_factory)));

    // Enumerate Adapters (find best GPU)
    if (FAILED(m_factory->EnumAdapterByGpuPreference(
            0, DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE, IID_PPV_ARGS(&m_adapter))))
    {
        // Fallback to default adapter
        m_factory->EnumAdapters1(0, &m_adapter);
    }

    // Create device
    DX12_CHECK(D3D12CreateDevice(m_adapter.Get(), D3D_FEATURE_LEVEL_12_1,
                                 IID_PPV_ARGS(&m_device)));

    // Create info queue (after device exists)
#ifdef _DEBUG
    m_infoQueue = std::make_unique<DX12InfoQueue>(m_device.Get());
#endif

    // TODO: Try 12_1, fallback to 12_0
    /*
      D3D_FEATURE_LEVEL featureLevels[] = {
          D3D_FEATURE_LEVEL_12_1,
          D3D_FEATURE_LEVEL_12_0
      };

      // Check what we got, enable DXR path if 12_1+
      if (featureLevel >= D3D_FEATURE_LEVEL_12_1) {
          // Use DXR pathtracer
      } else {
          // Use compute pathtracer
      }
    */

    // TODO: Check feature level support

    // Fill out a command queue desc
    D3D12_COMMAND_QUEUE_DESC queueDesc = {};
    // Direct command queue for graphics, compute, and copy operations
    queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
    // Normal priority
    queueDesc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;
    queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
    // Node mask for single GPU operation
    queueDesc.NodeMask = 0;

    // Create command queue
    DX12_CHECK(m_device->CreateCommandQueue(&queueDesc,
                                            IID_PPV_ARGS(&m_commandQueue)));
}

} // namespace pathtracer

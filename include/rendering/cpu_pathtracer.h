#pragma once

#include "core/dx12_info_queue.h"
#include "interfaces/pathtracer_interface.h"

#include <d3d12.h>

namespace pathtracer
{
class CpuPathtracer : public IPathTracer
{
  public:
    CpuPathtracer(ID3D12Device* device, DX12InfoQueue* infoQueue, UINT width, UINT height);
    ~CpuPathtracer() = default;

    auto Render(ID3D12GraphicsCommandList* commandList,
                ID3D12Resource* renderTarget,
                D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle) -> void override;

    auto Resize(const UINT width, const UINT height) -> void override;

    auto GetName() const -> const char* override
    {
        return "CPU Path Tracer";
    }

  private:
    ID3D12Device* m_device;     // Non-owning
    DX12InfoQueue* m_infoQueue; // Non-owning
    UINT m_width;
    UINT m_height;

    // TODO: remove once actual path tracing implementation is added
    const float m_clearColor[4] = {1.0f, 0.5f, 0.2f, 1.0f};
};

} // namespace pathtracer

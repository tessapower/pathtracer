#pragma once

#include "core/dx12_info_queue.h"
#include "interfaces/pathtracer_interface.h"

#include <d3d12.h>

namespace pathtracer
{
class CpuPathtracer : public IPathTracer
{
  public:
    /// <summary>
    /// Constructs a CPU-based path tracer. This implementation is intended for
    /// testing and debugging purposes, providing a reference implementation of
    /// a path tracing algorithm.
    /// </summary>
    /// <param name="device">The D3D12 device to use for resource creation.</param>
    /// <param name="infoQueue">The DX12 info queue for logging messages.</param>
    /// <param name="width">The initial width of the render target.</param>
    /// <param name="height">The initial height of the render target.</param>
    CpuPathtracer(ID3D12Device* device, DX12InfoQueue* infoQueue, UINT width,
                  UINT height);
    ~CpuPathtracer() = default;

    /// <summary>
    /// Renders the scene using the path tracing algorithm. This method should
    /// be called every frame to update the render target with the latest image.
    /// </summary>
    /// <param name="commandList">The command list to record rendering
    /// commands.</param>
    /// <param name="RenderTarget">UNUSED.</param>
    /// <param name="rtvHandle">The CPU descriptor handle for the render
    /// target.</param>
    /// <param name="camera">The camera defining the view for the current
    /// frame.</param>
    /// <param name="scene">The scene to be rendered.</param>
    auto Render(ID3D12GraphicsCommandList* commandList,
                ID3D12Resource* renderTarget,
                D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle) -> void override;

    /// <summary>
    /// Resizes internal resources to match the new width and height of the
    /// window. This should be called when the window is resized to ensure the
    /// path tracer continues to render correctly.
    /// </summary>
    auto Resize(const UINT width, const UINT height) -> void override;

    /// <summary>
    /// Returns the name of the path tracer for display in the UI.
    /// </summary>
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

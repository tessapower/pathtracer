#pragma once

#include <d3d12.h>

namespace pathtracer
{
class Camera;
class Scene;

class IPathTracer
{
  public:
    IPathTracer() = default;
    virtual ~IPathTracer() = default;

    /// <summary>
    /// Renders the scene using the path tracing algorithm. This method should
    /// be called every frame to update the render target with the latest image.
    /// </summary>
    /// <param name="commandList">The command list to record rendering
    /// commands.</param>
    /// <param name="RenderTarget">The render target to which the path tracer
    /// will output the image.</param>
    /// <param name="camera">The camera defining the view for the current
    /// frame.</param>
    /// <param name="scene">The scene to be rendered.</param>
    virtual auto Render(ID3D12GraphicsCommandList* commandList,
                        ID3D12Resource* renderTarget,
                        D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle
                        //, const Camera& camera,
                        // const Scene& scene
                        ) -> void = 0;

    /// <summary>
    /// Resizes internal resources to match the new width and height of the
    /// window. This should be called when the window is resized to ensure the
    /// path tracer continues to render correctly.
    /// </summary>
    virtual auto Resize(const UINT width, const UINT height) -> void = 0;

    /// <summary>
    /// Returns the name of the path tracer for display in the UI.
    /// </summary>
    virtual auto GetName() const -> const char* = 0;

    // TODO: add method to reset accumulation when camera moves or scene changes
    // virtual auto ResetAccumulation() -> void {}
};

} // namespace pathtracer

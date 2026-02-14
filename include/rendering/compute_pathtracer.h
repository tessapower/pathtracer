#pragma once

#include "core/dx12_info_queue.h"
#include "interfaces/pathtracer_interface.h"
#include "scene/camera.h"

#include <d3d12.h>

#include <wrl.h>

namespace pathtracer
{
class ComputePathtracer : public IPathTracer
{
  public:
    ComputePathtracer(ID3D12Device* device, DX12InfoQueue* infoQueue,
                      UINT width, UINT height);

    /// <summary>
    /// Renders the scene using the path tracing algorithm. This method should
    /// be called every frame to update the render target with the latest image.
    /// </summary>
    /// <param name="commandList">The command list to record rendering
    /// commands.</param>
    /// <param name="RenderTarget">The render target to which the path tracer
    /// will output the image.</param>
    /// <param name="rtvHandle">UNUSED.</param>
    /// <param name="camera">The camera defining the view for the current
    /// frame.</param>
    /// <param name="frameIdx">The index of the current frame.</param>
    auto Render(ID3D12GraphicsCommandList* commandList,
                ID3D12Resource* renderTarget,
                D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle,
                const Camera& camera,
                const UINT frameIdx) -> void override;

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
        return "Compute Shader Path Tracer";
    }

  private:
    auto LoadComputeShader() -> void;
    auto CreateRootSignature() -> void;
    auto CreatePipelineState() -> void;
    auto CreateOutputTexture() -> void;
    auto CreateDescriptorHeap() -> void;

    ID3D12Device* m_device;     // Non-owning
    DX12InfoQueue* m_infoQueue; // Non-owning
    UINT m_width;
    UINT m_height;

    // Root signature, PSO, output texture (UAV)
    Microsoft::WRL::ComPtr<ID3D12RootSignature> m_rootSignature;
    Microsoft::WRL::ComPtr<ID3D12PipelineState> m_pipelineState;
    Microsoft::WRL::ComPtr<ID3D12Resource> m_outputTexture;

    // Descriptor heap for UAV
    Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_descriptorHeap;
    UINT m_descriptorSize;

    // Shader bytecode loaded from file (used to create pipeline state)
    std::vector<uint8_t> m_shaderBytecode;
    static constexpr const char* m_COMPILED_SHADER_FILE =
        "shaders/simple_pathtracer.cso";
};

} // namespace pathtracer

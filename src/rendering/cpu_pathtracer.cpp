#include "stdafx.h"

#include "rendering/cpu_pathtracer.h"

namespace pathtracer
{
CpuPathtracer::CpuPathtracer(ID3D12Device* device, DX12InfoQueue* infoQueue,
                             UINT width, UINT height)
    : m_device(device), m_infoQueue(infoQueue), m_width(width), m_height(height)
{
}

auto CpuPathtracer::Render(ID3D12GraphicsCommandList* commandList,
                           ID3D12Resource* renderTarget,
                           D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle) -> void
{
    // Explicitly mark unused parameter to avoid warnings
    (void)renderTarget;

    commandList->ClearRenderTargetView(rtvHandle, m_clearColor, 0, nullptr);
}

auto CpuPathtracer::Resize(const UINT width, const UINT height) -> void
{
    m_width = width;
    m_height = height;
    // TODO: recreate CPU buffers and upload resources
}

} // namespace pathtracer

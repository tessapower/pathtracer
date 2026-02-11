#include "stdafx.h"

#include "core/application.h"
#include "core/dx12_device.h"
#include "core/window.h"
#include "rendering/renderer.h"

namespace pathtracer
{

Application::Application(const UINT width, const UINT height,
                         LPCTSTR title)
{
    m_width = width;
    m_height = height;
    m_title = title;
    Initialize();
}

Application::~Application()
{
    Shutdown();
}

int Application::Run()
{
    while (m_isRunning)
    {
        if (!m_window->ProcessMessages())
        {
            m_isRunning = false;
            break;
        }
        Tick();

        Sleep(1);
    }
    return 0;
}

void Application::Initialize()
{
    // Create window
    m_window = std::make_unique<Window>(m_width, m_height, m_title);

    // Create D3D12 device
    m_device = std::make_unique<DX12Device>();

    // Create renderer
    m_renderer = std::make_unique<Renderer>(
        m_device->GetDevice(), m_device->GetFactory(),
        m_device->GetCommandQueue(), m_window->GetHandle(),
        m_window->GetWidth(), m_window->GetHeight());

    // Set resize callback
    m_window->SetResizeCallback([this](UINT width, UINT height)
                                { OnResize(width, height); });

    // Show the window
    m_window->Show();

    // Start the application loop
    m_isRunning = true;
}

void Application::Tick()
{
    // TODO: Update game state
    m_renderer->RenderFrame();
}

void Application::Shutdown()
{
    // TODO: Wait for GPU to idle
    // TODO: Release all resources
    // TODO: Destroy window
}

void Application::OnResize(UINT width, UINT height)
{
    m_width = width;
    m_height = height;
    m_renderer->OnResize(m_width, m_height);
}

} // namespace pathtracer

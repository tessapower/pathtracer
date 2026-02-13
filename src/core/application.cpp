#include "stdafx.h"

#include "core/application.h"
#include "core/dx12_device.h"
#include "core/window.h"
#include "rendering/compute_pathtracer.h"
#include "rendering/cpu_pathtracer.h"
#include "rendering/renderer.h"

namespace pathtracer
{
Application::Application(const UINT width, const UINT height, LPCTSTR title)
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
    // Start the application loop
    m_isRunning = true;

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

    // Create pathtracer
    auto pathtracer = std::make_unique<ComputePathtracer>(
        m_device->GetDevice(), m_device->GetInfoQueue(), m_window->GetWidth(),
        m_window->GetHeight());

    // Create renderer
    m_renderer = std::make_unique<Renderer>(
        m_device->GetDevice(), m_device->GetFactory(),
        m_device->GetCommandQueue(), m_device->GetInfoQueue(), std::move(pathtracer),
        m_window->GetHandle(), m_window->GetWidth(), m_window->GetHeight());

    // Set resize callback
    m_window->SetResizeCallback([this](UINT width, UINT height)
                                { OnResize(width, height); });

    // Show the window
    m_window->Show();

    // Initialize performance tracking
    m_lastFrameTime = std::chrono::high_resolution_clock::now();
}

void Application::Tick()
{
    // Calculate delta time
    auto now = std::chrono::high_resolution_clock::now();
    m_deltaTime =
        std::chrono::duration<double, std::milli>(now - m_lastFrameTime).count();
    m_lastFrameTime = now;

    // Update frame count
    m_frameCount++;
    m_fpsUpdateTimer += m_deltaTime;

    // Update FPS every second
    if (m_fpsUpdateTimer >= 1000.0)
    {
        m_fps = m_frameCount / (m_fpsUpdateTimer / 1000.0);
        m_frameCount = 0;
        m_fpsUpdateTimer = 0.0;

        // Update window title with FPS and frame time
        TCHAR titleBuffer[256];
        _stprintf_s(titleBuffer, TEXT("DX12 Path Tracer | FPS: %.1f | Frame Time: %.2f ms"),
                    m_fps, 1000.0 / m_fps);
        m_window->SetTitle(titleBuffer);
    }

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

#include "stdafx.h"

#include "core/application.h"
#include "core/window.h"

#include <iostream>

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
    // TODO: Call Shutdown()
    std::cout << "Application destroyed\n";
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

        // Optional: Prevent 100% CPU usage when idle
        // Remove this once you have a proper render loop
        Sleep(1); // or use VSync with Present
    }
    return 0;
}

void Application::Initialize()
{
    // Create window
    m_window = std::make_unique<Window>(m_width, m_height, m_title);

    // Set resize callback
    m_window->SetResizeCallback([this](UINT width, UINT height)
                                { OnResize(width, height); });

    // TODO: Initialize DX12 device
    // TODO: Create swap chain
    // TODO: Set up initial rendering resources

    // Show the window
    m_window->Show();

    // Start the application loop
    m_isRunning = true;
}

void Application::Tick()
{
    // TODO: Update game state
    // TODO: Record render commands
    // TODO: Execute commands
    // TODO: Present frame
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

    // TODO: Resize swap chain
    // TODO: Update viewport
    // TODO: Recreate size-dependent resources
}

} // namespace pathtracer

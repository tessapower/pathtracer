#ifndef APPLICATION_H
#define APPLICATION_H

#include "platform/windows_fwd.h"

#include <chrono>
#include <memory>

// Forward declarations
namespace pathtracer
{
class Window;
class DX12Device;
class Renderer;
} // namespace pathtracer

namespace pathtracer
{
class Application
{
  public:
    /// <summary>
    /// Main application class - handles initialization, main loop, and shutdown
    /// <para></para>
    /// Responsibilities:
    /// <para>- Create and manage window</para>
    /// <para>- Initialize DX12 device and swap chain</para>
    /// <para>- Run main application loop</para>
    /// <para>- Handle cleanup on exit</para>
    /// <para>- Propagate exceptions with meaningful messages</para>
    /// </summary>
    Application(const UINT width = 960, const UINT height = 540,
                LPCTSTR window_title = TEXT("DX12 Path Tracer"));
    ~Application();

    // Disable copy/move

    Application(const Application&) = delete;
    Application& operator=(const Application&) = delete;

    /// <summary>
    /// Run the application main loop
    /// <para></para>
    /// Main loop structure:
    /// <para>- Process Windows messages</para>
    /// <para>- Update scene/camera</para>
    /// <para>- Render frame</para>
    /// <para>- Present to swap chain</para>
    /// <para>- Repeat until quit</para>
    /// </summary>
    /// <returns>Exit code (0 for success)</returns>
    auto Run() -> int;

  private:
    /// <summary>
    /// Initialize all subsystems
    /// <para>- Create window</para>
    /// <para>- Initialize DX12 device</para>
    /// <para>- Create swap chain</para>
    /// <para>- Set up rendering resources</para>
    /// </summary>
    auto Initialize() -> void;

    /// <summary>
    /// Main render loop iteration
    /// <para>- Update frame state</para>
    /// <para>- Record commands</para>
    /// <para>- Execute rendering</para>
    /// <para>- Present frame</para>
    /// </summary>
    auto Tick() -> void;

    /// <summary>
    /// Clean shutdown of all resources
    /// <para>- Wait for GPU to finish</para>
    /// <para>- Release all COM objects</para>
    /// <para>- Destroy window</para>
    /// </summary>
    auto Shutdown() -> void;

    /// <summary>
    /// Handle window resize events
    /// <para>- Resize swap chain buffers</para>
    /// <para>- Update viewport</para>
    /// <para>- Recreate render targets</para>
    /// </summary>
    auto OnResize(UINT width, UINT height) -> void;

  private:
    std::unique_ptr<Window> m_window;
    std::unique_ptr<DX12Device> m_device;
    std::unique_ptr<Renderer> m_renderer;
    bool m_isRunning = false;
    UINT m_width = 0;
    UINT m_height = 0;
    LPCTSTR m_title;

    // Performance tracking
    std::chrono::high_resolution_clock::time_point m_lastFrameTime;
    double m_deltaTime = 0.0;
    double m_fps = 0.0;
    UINT m_frameCount = 0;
    double m_fpsUpdateTimer = 0.0;
};

} // namespace pathtracer

#endif

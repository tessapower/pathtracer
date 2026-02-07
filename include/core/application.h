#ifndef APPLICATION_H
#define APPLICATION_H

#include "platform/windows_fwd.h"

#include <memory>
#include <string>

// Forward declarations
namespace pathtracer
{
class Window;
class DX12Device;
class SwapChain;
} // namespace pathtracer

namespace pathtracer
{
/**
 * @brief Main application class - handles initialization, main loop, and
 * shutdown
 *
 * Responsibilities:
 * - Create and manage window
 * - Initialize DX12 device and swap chain
 * - Run main application loop
 * - Handle cleanup on exit
 * - Propagate exceptions with meaningful messages
 *
 * Usage:
 *   Application app(1920, 1080, "My Path Tracer");
 *   return app.Run();
 */
class Application
{
  public:
    Application(const UINT width = 960, const UINT height = 540,
                LPCTSTR window_title = TEXT("DX12 Path Tracer"));
    ~Application();

    // Disable copy/move
    Application(const Application&) = delete;
    Application& operator=(const Application&) = delete;

    /**
     * @brief Run the application main loop
     * @return Exit code (0 for success)
     *
     * Main loop structure:
     * - Process Windows messages
     * - Update scene/camera
     * - Render frame
     * - Present to swap chain
     * - Repeat until quit
     */
    auto Run() -> int;

  private:
    /**
     * @brief Initialize all subsystems
     * - Create window
     * - Initialize DX12 device
     * - Create swap chain
     * - Set up rendering resources
     */
    auto Initialize() -> void;

    /**
     * @brief Main render loop iteration
     * - Update frame state
     * - Record commands
     * - Execute rendering
     * - Present frame
     */
    auto Tick() -> void;

    /**
     * @brief Clean shutdown of all resources
     * - Wait for GPU to finish
     * - Release all COM objects
     * - Destroy window
     */
    auto Shutdown() -> void;

    /**
     * @brief Handle window resize events
     * - Resize swap chain buffers
     * - Update viewport
     * - Recreate render targets
     */
    auto OnResize(UINT width, UINT height) -> void;

  private:
    std::unique_ptr<Window> m_window;
    // std::unique_ptr<DX12Device> m_device;
    // std::unique_ptr<SwapChain> m_swapChain;
    bool m_isRunning = false;
    UINT m_width = 0;
    UINT m_height = 0;
    LPCTSTR m_title;
};

} // namespace pathtracer

#endif

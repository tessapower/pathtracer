#pragma once

#include "platform/windows_fwd.h"

#include <functional>
#include <string>

namespace pathtracer
{
/// <summary>
/// Win32 window wrapper
/// <para></para>
/// Encapsulates Win32 window creation and message handling.
/// Provides callbacks for resize, input, and other events.
/// </summary>
class Window
{
  public:
    using ResizeCallback = std::function<void(UINT width, UINT height)>;

    Window(UINT width, UINT height, LPCTSTR title);
    ~Window();

    // Disable copy/move constructors and assignment
    Window(const Window&) = delete;
    Window& operator=(const Window&) = delete;

    /// <summary>
    /// Process Windows messages (non-blocking)
    /// </summary>
    /// <returns>false if WM_QUIT received, true otherwise</returns>
    auto ProcessMessages() -> bool;

    /// <summary>
    /// Returns the window handle for DX12 swap chain creation
    /// </summary>
    auto GetHandle() const -> HWND
    {
        return m_hwnd;
    }

    /// <summary>
    /// Returns the current window width
    /// </summary>
    auto GetWidth() const -> UINT
    {
        return m_width;
    }

    /// <summary>
    /// Returns the current window height
    /// </summary>
    auto GetHeight() const -> UINT
    {
        return m_height;
    }

    /// <summary>
    /// Set callback for window resize events
    /// </summary>
    auto SetResizeCallback(ResizeCallback callback) -> void
    {
        m_resizeCallback = callback;
    }

    /// <summary>
    /// Show the window
    /// </summary>
    auto Show() -> void;

  private:
    static auto WINAPI HandleMsgSetup(HWND hwnd, UINT uMsg, WPARAM wParam,
                                      LPARAM lParam) noexcept -> LRESULT;
    static auto WINAPI MsgThunk(HWND hwnd, UINT uMsg, WPARAM wParam,
                                LPARAM lParam) noexcept -> LRESULT;
    auto HandleMsg(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) noexcept
        -> LRESULT;

    auto OnResize(int width, int height) -> void;
    auto OnKeyDown(int key) -> void;
    auto OnKeyUp(int key) -> void;
    auto OnMouseMove(int x, int y) -> void;

  private:
    HWND m_hwnd = nullptr;
    HINSTANCE m_hinstance = nullptr;
    UINT m_width = 0;
    UINT m_height = 0;
    LPCTSTR m_className = TEXT("PathTracerWindowClass");
    ResizeCallback m_resizeCallback;
};

} // namespace pathtracer

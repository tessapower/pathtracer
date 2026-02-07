#include "stdafx.h"

#include "core/window.h"

#include <cassert>
#include <iostream>
#include <string>

namespace pathtracer
{
Window::Window(UINT width, UINT height, LPCTSTR title)
    : m_width(width), m_height(height), m_hinstance(GetModuleHandle(nullptr))
{
    constexpr DWORD WINDOW_STYLE =
        WS_OVERLAPPED | WS_CAPTION | WS_MINIMIZEBOX | WS_SYSMENU;
    constexpr DWORD WINDOW_EX_STYLE = WS_EX_OVERLAPPEDWINDOW;

    // Register window class
    WNDCLASSEX wc = {};
    wc.cbSize = sizeof(WNDCLASSEX);
    wc.style = CS_CLASSDC;
    wc.lpfnWndProc = &Window::HandleMsgSetup;
    wc.hInstance = m_hinstance;
    wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wc.lpszClassName = m_className;

    const ATOM atom = RegisterClassEx(&wc);
    if (atom == 0)
    {
        DWORD error = GetLastError();

        // ERROR_CLASS_ALREADY_EXISTS (1410) is common - not necessarily fatal
        if (error == ERROR_CLASS_ALREADY_EXISTS)
        {
            // Class already registered (maybe from previous run) - this is OK
            OutputDebugString(TEXT("Window class already registered\n"));
        }
        else
        {
            // Actual error - throw exception
            throw std::runtime_error("Failed to register window class: " +
                                     std::to_string(error));
        }
    }

    // Calculate window size (includes borders and title bar)
    RECT viewport{0, 0, static_cast<long>(width), static_cast<long>(height)};
    AdjustWindowRectEx(&viewport, WINDOW_STYLE, false, WINDOW_EX_STYLE);

    // Create window
    m_hwnd = CreateWindowEx(
        WINDOW_EX_STYLE,                // Extended window style(s)
        m_className,                    // Window class name
        title,                          // Window name in title bar
        WINDOW_STYLE,                   // Window style
        CW_USEDEFAULT,                  // x position of window
        CW_USEDEFAULT,                  // y position of window
        viewport.right - viewport.left, // Client width
        viewport.bottom - viewport.top, // Client height
        nullptr,                        // Handle to parent window
        nullptr,                        // Handle to menu
        m_hinstance, // Handle to instance to be associated with window
        this         // Pass a pointer to this instance of Window and
                     // be able to access it from the created h_wnd
    );

    // Handle creation failure
    if (!m_hwnd)
    {
        DWORD error = GetLastError();
        throw std::runtime_error("Failed to create window: " +
                                 std::to_string(error));
    }

    // Note: title is LPCTSTR (wide string when UNICODE is defined)
    // For console output, we'd need conversion, so using OutputDebugString instead
    OutputDebugString(TEXT("Window created\n"));
}

Window::~Window()
{
    // Destroy window
    if (m_hwnd)
    {
        DestroyWindow(m_hwnd);
        UnregisterClass(m_className, m_hinstance);
    }
    std::cout << "Window destroyed\n";
}

bool Window::ProcessMessages()
{
    // Process all pending messages (non-blocking)
    MSG msg = {};
    while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
    {
        if (msg.message == WM_QUIT)
        {
            return false;
        }
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    return true;
}

void Window::Show()
{
    ShowWindow(m_hwnd, SW_SHOWDEFAULT);
    UpdateWindow(m_hwnd);
}

auto __stdcall Window::HandleMsgSetup(HWND hwnd, UINT uMsg, WPARAM wParam,
                                      LPARAM lParam) noexcept -> LRESULT
{
    if (uMsg == WM_NCCREATE)
    {
        // Retrieve the lpParam we passed in when creating the hWnd
        const CREATESTRUCT* pCreate = reinterpret_cast<CREATESTRUCT*>(lParam);
        auto pWindow = static_cast<Window*>(pCreate->lpCreateParams);

        // Set the USERDATA to point to this window
        SetWindowLongPtr(hwnd, GWLP_USERDATA,
                         reinterpret_cast<LONG_PTR>(pWindow));
        // Now set the WNDPROC to point to handleMsgThunk
        SetWindowLongPtr(hwnd, GWLP_WNDPROC,
                         reinterpret_cast<LONG_PTR>(&Window::MsgThunk));

        return pWindow->HandleMsg(hwnd, uMsg, wParam, lParam);
    }

    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

auto __stdcall Window::MsgThunk(HWND hwnd, UINT uMsg, WPARAM wParam,
                                LPARAM lParam) noexcept -> LRESULT
{
    // Get a pointer to the window associated with the given h_wnd
    const auto pWindow =
        reinterpret_cast<Window*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));

    // Forward on the message to the Window instance
    return pWindow->HandleMsg(hwnd, uMsg, wParam, lParam);
}

auto Window::HandleMsg(HWND hwnd, UINT uMsg, WPARAM wParam,
                       LPARAM lParam) noexcept -> LRESULT
{
    switch (uMsg)
    {
    case WM_SIZE: // Sent when the user resizes the window
    {
        if (wParam == SIZE_MINIMIZED)
            return 0;

        // Queue resize
        OnResize(static_cast<UINT>(LOWORD(lParam)),
                 static_cast<UINT>(HIWORD(lParam)));

        return 0;
    }

    case WM_KEYDOWN:
    {
        OnKeyDown(static_cast<int>(wParam));

        return 0;
    }

    case WM_KEYUP:
    {
        OnKeyUp(static_cast<int>(wParam));

        return 0;
    }

    case WM_MOUSEMOVE:
    {
        int x = LOWORD(lParam);
        int y = HIWORD(lParam);
        OnMouseMove(x, y);

        return 0;
    }

    case WM_DESTROY: // Sent when the window is being destroyed
    case WM_CLOSE:   // Sent when the window is being closed
    case WM_QUIT:    // Sent when the application is quitting
    {
        PostQuitMessage(0);

        return 0;
    }

    case WM_NCDESTROY: // Sent when the non-client area of the window is being
                       // destroyed (after WM_DESTROY)
    {
        // Restore original window procedure
        SetWindowLongPtr(hwnd, GWLP_USERDATA, 0);
        PostQuitMessage(0);

        return 0;
    }

    case WM_PAINT: // Windows accumulates invalid regions if unhandled
    {
        PAINTSTRUCT ps;
        BeginPaint(hwnd, &ps);
        // Do nothing, DX12 handles rendering
        EndPaint(hwnd, &ps);

        return 0;
    }

    case WM_KILLFOCUS: // Sent when the window loses keyboard focus
    default:
        return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }
}

void Window::OnResize(int width, int height)
{
    // Update stored dimensions
    m_width = width;
    m_height = height;

    // Call resize callback if set
    if (m_resizeCallback)
    {
        m_resizeCallback(width, height);
    }
}

void Window::OnKeyDown(int key)
{
    UNREFERENCED_PARAMETER(key);
    OutputDebugString(TEXT("Key down\n"));
    // TODO: Handle key presses
    // ESC to quit?
    // WASD for camera movement?
}

void Window::OnKeyUp(int key)
{
    UNREFERENCED_PARAMETER(key);
    OutputDebugString(TEXT("Key up\n"));
    // TODO: Handle key releases
}

void Window::OnMouseMove(int x, int y)
{
    UNREFERENCED_PARAMETER(x);
    UNREFERENCED_PARAMETER(y);
    OutputDebugString(TEXT("Mouse move\n"));
    // TODO: Handle mouse movement for camera control
}

} // namespace pathtracer

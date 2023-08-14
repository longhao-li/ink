#include "ink/core/window.hpp"
#include "ink/core/exception.hpp"
#include "ink/render/device.hpp"

#include <shellapi.h>
#include <windowsx.h>

using namespace ink;

namespace {

/// @brief
///   Convert ink window style to win32 window style flags.
///
/// @param flags
///   The ink window style to be converted.
/// @param[out] dwStyle
///   Win32 window style flags.
/// @param[out] dwStyleEx
///   Win32 window extented style flags.
auto toWin32Style(WindowStyle flags, DWORD &dwStyle, DWORD &dwStyleEx) noexcept -> void {
    dwStyle   = 0;
    dwStyleEx = WS_EX_ACCEPTFILES;

    if ((flags & WindowStyle::Titled) != WindowStyle::None) {
        dwStyle |= (WS_CAPTION | WS_OVERLAPPED | WS_SYSMENU);
        dwStyleEx |= WS_EX_APPWINDOW;

        if ((flags & WindowStyle::Resizable) != WindowStyle::None)
            dwStyle |= (WS_SIZEBOX | WS_MINIMIZEBOX | WS_MAXIMIZEBOX);
    } else {
        dwStyle |= WS_POPUP;
        if ((flags & WindowStyle::Resizable) != WindowStyle::None)
            dwStyle |= WS_SIZEBOX;
    }

    if ((flags & WindowStyle::Topmost) != WindowStyle::None)
        dwStyleEx |= WS_EX_TOPMOST;
}

/// @brief
///   Convert a UTF-8 string to a wide string.
///
/// @param str
///   The UTF-8 string to be converted.
[[nodiscard]] auto toWideString(std::string_view str) noexcept -> std::wstring {
    const int count =
        MultiByteToWideChar(CP_UTF8, 0, str.data(), static_cast<int>(str.size()), nullptr, 0);
    if (count <= 0)
        return {};

    std::wstring result;
    result.resize(static_cast<std::size_t>(count));
    MultiByteToWideChar(CP_UTF8, 0, str.data(), static_cast<int>(str.size()), result.data(), count);
    return result;
}

/// @brief
///   Convert a wide string to UTF-8 string.
///
/// @param str
///   The wide string to be converted.
[[nodiscard]] auto toUTF8String(std::wstring_view str) noexcept -> std::string {
    const int count = WideCharToMultiByte(CP_UTF8, 0, str.data(), static_cast<int>(str.size()),
                                          nullptr, 0, nullptr, nullptr);
    if (count <= 0)
        return {};

    std::string result;
    result.resize(static_cast<std::size_t>(count));
    WideCharToMultiByte(CP_UTF8, 0, str.data(), static_cast<int>(str.size()), result.data(), count,
                        nullptr, nullptr);
    return result;
}

/// @brief
///   Get modifier keys status.
///
/// @return
///   Modifier keys status.
[[nodiscard]] auto modifiers() noexcept -> Modifier {
    Modifier mods = Modifier::None;

    if (GetKeyState(VK_SHIFT) & 0x8000)
        mods |= Modifier::Shift;
    if (GetKeyState(VK_CONTROL) & 0x8000)
        mods |= Modifier::Ctrl;
    if (GetKeyState(VK_MENU) & 0x8000)
        mods |= Modifier::Alt;
    if ((GetKeyState(VK_LWIN) | GetKeyState(VK_RWIN)) & 0x8000)
        mods |= Modifier::Super;

    return mods;
}

/// @brief
///   Key code map. Used to convert win32 virtual key to ink key code.
constexpr const std::array<KeyCode, 0x100> KEY_CODE_MAP = {
    KeyCode::Undefined,   KeyCode::MouseLeft,   KeyCode::MouseRight,
    KeyCode::Break,       KeyCode::MouseMiddle, KeyCode::MouseX1,
    KeyCode::MouseX2,     KeyCode::Undefined,   KeyCode::Backspace,
    KeyCode::Tab,         KeyCode::Undefined,   KeyCode::Undefined,
    KeyCode::Clear,       KeyCode::Enter,       KeyCode::Undefined,
    KeyCode::Undefined,   KeyCode::Undefined,   KeyCode::Undefined,
    KeyCode::Undefined,   KeyCode::Pause,       KeyCode::CapsLock,
    KeyCode::Undefined,   KeyCode::Undefined,   KeyCode::Undefined,
    KeyCode::Undefined,   KeyCode::Undefined,   KeyCode::Undefined,
    KeyCode::Escape,      KeyCode::Undefined,   KeyCode::Undefined,
    KeyCode::Undefined,   KeyCode::Undefined,   KeyCode::Space,
    KeyCode::PageUp,      KeyCode::PageDown,    KeyCode::End,
    KeyCode::Home,        KeyCode::Left,        KeyCode::Up,
    KeyCode::Right,       KeyCode::Down,        KeyCode::Select,
    KeyCode::Print,       KeyCode::Execute,     KeyCode::PrintScreen,
    KeyCode::Insert,      KeyCode::Delete,      KeyCode::Help,
    KeyCode::Digit0,      KeyCode::Digit1,      KeyCode::Digit2,
    KeyCode::Digit3,      KeyCode::Digit4,      KeyCode::Digit5,
    KeyCode::Digit6,      KeyCode::Digit7,      KeyCode::Digit8,
    KeyCode::Digit9,      KeyCode::Undefined,   KeyCode::Undefined,
    KeyCode::Undefined,   KeyCode::Undefined,   KeyCode::Undefined,
    KeyCode::Undefined,   KeyCode::Undefined,   KeyCode::A,
    KeyCode::B,           KeyCode::C,           KeyCode::D,
    KeyCode::E,           KeyCode::F,           KeyCode::G,
    KeyCode::H,           KeyCode::I,           KeyCode::J,
    KeyCode::K,           KeyCode::L,           KeyCode::M,
    KeyCode::N,           KeyCode::O,           KeyCode::P,
    KeyCode::Q,           KeyCode::R,           KeyCode::S,
    KeyCode::T,           KeyCode::U,           KeyCode::V,
    KeyCode::W,           KeyCode::X,           KeyCode::Y,
    KeyCode::Z,           KeyCode::LeftSuper,   KeyCode::RightSuper,
    KeyCode::ContextMenu, KeyCode::Undefined,   KeyCode::Sleep,
    KeyCode::Keypad0,     KeyCode::Keypad1,     KeyCode::Keypad2,
    KeyCode::Keypad3,     KeyCode::Keypad4,     KeyCode::Keypad5,
    KeyCode::Keypad6,     KeyCode::Keypad7,     KeyCode::Keypad8,
    KeyCode::Keypad9,     KeyCode::Multiply,    KeyCode::Add,
    KeyCode::Undefined,   KeyCode::Subtract,    KeyCode::Decimal,
    KeyCode::Divide,      KeyCode::F1,          KeyCode::F2,
    KeyCode::F3,          KeyCode::F4,          KeyCode::F5,
    KeyCode::F6,          KeyCode::F7,          KeyCode::F8,
    KeyCode::F9,          KeyCode::F10,         KeyCode::F11,
    KeyCode::F12,         KeyCode::F13,         KeyCode::F14,
    KeyCode::F15,         KeyCode::F16,         KeyCode::F17,
    KeyCode::F18,         KeyCode::F19,         KeyCode::F20,
    KeyCode::F21,         KeyCode::F22,         KeyCode::F23,
    KeyCode::F24,         KeyCode::Undefined,   KeyCode::Undefined,
    KeyCode::Undefined,   KeyCode::Undefined,   KeyCode::Undefined,
    KeyCode::Undefined,   KeyCode::Undefined,   KeyCode::Undefined,
    KeyCode::NumLock,     KeyCode::ScrollLock,  KeyCode::Undefined,
    KeyCode::Undefined,   KeyCode::Undefined,   KeyCode::Undefined,
    KeyCode::Undefined,   KeyCode::Undefined,   KeyCode::Undefined,
    KeyCode::Undefined,   KeyCode::Undefined,   KeyCode::Undefined,
    KeyCode::Undefined,   KeyCode::Undefined,   KeyCode::Undefined,
    KeyCode::Undefined,   KeyCode::LeftShift,   KeyCode::RightShift,
    KeyCode::LeftCtrl,    KeyCode::RightCtrl,   KeyCode::LeftAlt,
    KeyCode::RightAlt,    KeyCode::Undefined,   KeyCode::Undefined,
    KeyCode::Undefined,   KeyCode::Undefined,   KeyCode::Undefined,
    KeyCode::Undefined,   KeyCode::Undefined,   KeyCode::Undefined,
    KeyCode::Undefined,   KeyCode::Undefined,   KeyCode::Undefined,
    KeyCode::Undefined,   KeyCode::Undefined,   KeyCode::Undefined,
    KeyCode::Undefined,   KeyCode::Undefined,   KeyCode::Undefined,
    KeyCode::Undefined,   KeyCode::Undefined,   KeyCode::Undefined,
    KeyCode::Semicolon,   KeyCode::Equal,       KeyCode::Comma,
    KeyCode::Minus,       KeyCode::Period,      KeyCode::Slash,
    KeyCode::BackQuote,   KeyCode::Undefined,   KeyCode::Undefined,
    KeyCode::Undefined,   KeyCode::Undefined,   KeyCode::Undefined,
    KeyCode::Undefined,   KeyCode::Undefined,   KeyCode::Undefined,
    KeyCode::Undefined,   KeyCode::Undefined,   KeyCode::Undefined,
    KeyCode::Undefined,   KeyCode::Undefined,   KeyCode::Undefined,
    KeyCode::Undefined,   KeyCode::Undefined,   KeyCode::Undefined,
    KeyCode::Undefined,   KeyCode::Undefined,   KeyCode::Undefined,
    KeyCode::Undefined,   KeyCode::Undefined,   KeyCode::Undefined,
    KeyCode::Undefined,   KeyCode::Undefined,   KeyCode::Undefined,
    KeyCode::LeftBracket, KeyCode::BackSlash,   KeyCode::RightBracket,
    KeyCode::Quote,       KeyCode::Undefined,   KeyCode::Undefined,
    KeyCode::Undefined,   KeyCode::Undefined,   KeyCode::Undefined,
    KeyCode::Undefined,   KeyCode::Undefined,   KeyCode::Undefined,
    KeyCode::Undefined,   KeyCode::Undefined,   KeyCode::Undefined,
    KeyCode::Undefined,   KeyCode::Undefined,   KeyCode::Undefined,
    KeyCode::Undefined,   KeyCode::Undefined,   KeyCode::Undefined,
    KeyCode::Undefined,   KeyCode::Undefined,   KeyCode::Undefined,
    KeyCode::Undefined,   KeyCode::Undefined,   KeyCode::Undefined,
    KeyCode::Undefined,   KeyCode::Undefined,   KeyCode::Undefined,
    KeyCode::Undefined,   KeyCode::Undefined,   KeyCode::Undefined,
    KeyCode::Undefined,   KeyCode::Undefined,   KeyCode::Undefined,
    KeyCode::Undefined,
};

} // namespace

ink::Window::Window(std::string_view title,
                    std::uint32_t    width,
                    std::uint32_t    height,
                    WindowStyle      style)
    : m_hInstance(GetModuleHandleW(nullptr)),
      m_class(),
      m_hWnd(),
      m_title(title),
      m_windowWidth(),
      m_windowHeight(),
      m_frameWidth(width),
      m_frameHeight(height),
      m_isMinimized(false),
      m_highSurrogate(),
      m_focusCallback(),
      m_charCallback(),
      m_keyCallback(),
      m_mousePosCallback(),
      m_mouseWheelCallback(),
      m_resizeCallback(),
      m_minimizeCallback(),
      m_restoreCallback(),
      m_maximizeCallback(),
      m_windowPosCallback(),
      m_fileDropCallback() {
    // Setup DPI awareness. It is safe to call for multi-times.
    SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);

    std::wstring wideTitle(toWideString(title));

    { // Register window class.
        const WNDCLASSEXW clsEx{
            /* cbSize        = */ sizeof(WNDCLASSEX),
            /* style         = */ CS_HREDRAW | CS_VREDRAW,
            /* lpfnWndProc   = */ &Window::windowProc,
            /* cbClsExtra    = */ 0,
            /* cbWndExtra    = */ sizeof(void *),
            /* hInstance     = */ m_hInstance,
            /* hIcon         = */ nullptr,
            /* hCursor       = */ LoadCursorW(nullptr, IDC_ARROW),
            /* hbrBackground = */ reinterpret_cast<HBRUSH>(COLOR_WINDOW + 1),
            /* lpszMenuName  = */ nullptr,
            /* lpszClassName = */ wideTitle.c_str(),
            /* hIconSm       = */ nullptr,
        };

        m_class = RegisterClassExW(&clsEx);
        if (m_class == 0)
            throw SystemErrorException(static_cast<std::int32_t>(GetLastError()),
                                       "Failed to register win32 class for window.");
    }

    // Create window.
    DWORD dwStyle, dwStyleEx;
    toWin32Style(style, dwStyle, dwStyleEx);

    RECT rect{0, 0, static_cast<LONG>(width), static_cast<LONG>(height)};
    AdjustWindowRectEx(&rect, dwStyle, FALSE, dwStyleEx);
    m_windowWidth  = static_cast<std::uint32_t>(rect.right - rect.left);
    m_windowHeight = static_cast<std::uint32_t>(rect.bottom - rect.top);

    m_hWnd = CreateWindowExW(dwStyleEx, reinterpret_cast<LPCWSTR>(m_class), wideTitle.c_str(),
                             dwStyle, CW_USEDEFAULT, CW_USEDEFAULT, static_cast<int>(m_windowWidth),
                             static_cast<int>(m_windowHeight), nullptr, nullptr, m_hInstance, this);
    if (m_hWnd == nullptr) {
        DWORD errc = GetLastError();
        UnregisterClassW(reinterpret_cast<LPCWSTR>(m_class), m_hInstance);
        throw SystemErrorException(static_cast<std::int32_t>(errc),
                                   "Failed to create win32 window.");
    }

    ShowWindowAsync(m_hWnd, SW_SHOW);
}

ink::Window::Window(Window &&other) noexcept
    : m_hInstance(other.m_hInstance),
      m_class(other.m_class),
      m_hWnd(other.m_hWnd),
      m_title(std::move(other.m_title)),
      m_windowWidth(other.m_windowWidth),
      m_windowHeight(other.m_windowHeight),
      m_frameWidth(other.m_frameWidth),
      m_frameHeight(other.m_frameHeight),
      m_isMinimized(other.m_isMinimized),
      m_highSurrogate(other.m_highSurrogate),
      m_focusCallback(std::move(other.m_focusCallback)),
      m_closeCallback(std::move(other.m_closeCallback)),
      m_charCallback(std::move(other.m_charCallback)),
      m_keyCallback(std::move(other.m_keyCallback)),
      m_mousePosCallback(std::move(other.m_mousePosCallback)),
      m_mouseWheelCallback(std::move(other.m_mouseWheelCallback)),
      m_resizeCallback(std::move(other.m_resizeCallback)),
      m_minimizeCallback(std::move(other.m_minimizeCallback)),
      m_restoreCallback(std::move(other.m_restoreCallback)),
      m_maximizeCallback(std::move(other.m_maximizeCallback)),
      m_windowPosCallback(std::move(other.m_windowPosCallback)),
      m_fileDropCallback(std::move(other.m_fileDropCallback)) {
    if (m_hWnd != nullptr && IsWindow(m_hWnd))
        SetWindowLongPtrW(m_hWnd, 0, reinterpret_cast<LONG_PTR>(this));

    other.m_hInstance = nullptr;
    other.m_class     = 0;
    other.m_hWnd      = nullptr;
}

ink::Window::~Window() noexcept {
    if (m_hWnd != nullptr) {
        DestroyWindow(m_hWnd);
        UnregisterClassW(reinterpret_cast<LPCWSTR>(m_class), m_hInstance);
    }
}

auto ink::Window::resize(WindowSize frame) noexcept -> void {
    if (m_hWnd == nullptr)
        return;

    RECT rect{0, 0, static_cast<LONG>(frame.width), static_cast<LONG>(frame.height)};
    AdjustWindowRectEx(&rect, static_cast<DWORD>(GetWindowLongW(m_hWnd, GWL_STYLE)), FALSE,
                       static_cast<DWORD>(GetWindowLongW(m_hWnd, GWL_EXSTYLE)));

    SetWindowPos(m_hWnd, nullptr, 0, 0, rect.right - rect.left, rect.bottom - rect.top,
                 SWP_ASYNCWINDOWPOS | SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE);

    m_frameWidth   = frame.width;
    m_frameHeight  = frame.height;
    m_windowWidth  = static_cast<std::uint32_t>(rect.right - rect.left);
    m_windowHeight = static_cast<std::uint32_t>(rect.bottom - rect.top);
}

auto ink::Window::center() noexcept -> void {
    if (m_hWnd == nullptr)
        return;

    RECT rect;
    GetWindowRect(m_hWnd, &rect);

    HMONITOR    monitor = MonitorFromWindow(m_hWnd, MONITOR_DEFAULTTONEAREST);
    MONITORINFO monitorInfo{};
    monitorInfo.cbSize = sizeof(monitorInfo);
    GetMonitorInfoW(monitor, &monitorInfo);

    RECT screenRect = monitorInfo.rcWork;

    LONG halfScreenWidth  = (screenRect.right - screenRect.left) / 2;
    LONG halfScreenHeight = (screenRect.bottom - screenRect.top) / 2;

    LONG halfWindowWidth  = (rect.right - rect.left) / 2;
    LONG halfWindowHeight = (rect.bottom - rect.top) / 2;

    LONG x = halfScreenWidth - halfWindowWidth;
    LONG y = halfScreenHeight - halfWindowHeight;

    SetWindowPos(m_hWnd, nullptr, x, y, -1, -1,
                 SWP_ASYNCWINDOWPOS | SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE);
}

auto CALLBACK ink::Window::windowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
    -> LRESULT {
    auto *window = reinterpret_cast<Window *>(GetWindowLongPtrW(hWnd, 0));
    switch (message) {
    case WM_NCCREATE: {
        auto *cs = reinterpret_cast<CREATESTRUCTW *>(lParam);
        SetWindowLongPtrW(hWnd, 0, reinterpret_cast<LONG_PTR>(cs->lpCreateParams));
        return DefWindowProcW(hWnd, message, wParam, lParam);
    }

    case WM_SETFOCUS: {
        if (window->m_focusCallback)
            window->m_focusCallback(*window, true);
        return 0;
    }

    case WM_KILLFOCUS: {
        if (window->m_focusCallback)
            window->m_focusCallback(*window, false);
        return 0;
    }

    case WM_CLOSE: {
        if (window->m_closeCallback)
            window->m_closeCallback(*window);

        DestroyWindow(hWnd);
        UnregisterClassW(reinterpret_cast<LPCWSTR>(window->m_class), window->m_hInstance);

        window->m_hWnd  = nullptr;
        window->m_class = 0;

        return 0;
    }

    case WM_CHAR:
        [[fallthrough]];
    case WM_SYSCHAR: {
        if (wParam >= 0xD800 && wParam <= 0xDBFF) {
            window->m_highSurrogate = static_cast<WCHAR>(wParam);
            return 0;
        }

        if (window->m_charCallback) {
            char32_t codePoint = 0;
            if (wParam >= 0xDC00 && wParam <= 0xDFFF) {
                if (window->m_highSurrogate) {
                    codePoint += (window->m_highSurrogate - 0xD800U) << 10;
                    codePoint += static_cast<WCHAR>(wParam) - 0xDC00U;
                    codePoint += 0x10000U;
                }
            } else {
                codePoint = static_cast<WCHAR>(wParam);
            }

            // Handle char callback.
            window->m_charCallback(*window, codePoint);
        }

        return 0;
    }

    case WM_UNICHAR: {
        if (wParam == UNICODE_NOCHAR)
            return TRUE;

        if (window->m_charCallback)
            window->m_charCallback(*window, static_cast<char32_t>(wParam));

        return 0;
    }

    case WM_KEYDOWN:
        [[fallthrough]];
    case WM_SYSKEYDOWN:
        [[fallthrough]];
    case WM_KEYUP:
        [[fallthrough]];
    case WM_SYSKEYUP: {
        if (window->m_keyCallback) {
            const auto action = (HIWORD(lParam) & KF_UP) ? KeyAction::Release : KeyAction::Press;
            const auto mods   = modifiers();

            KeyCode key;

            UINT scanCode = (lParam & 0x00FF0000) >> 16;
            bool extended = (lParam & 0x01000000) != 0;

            switch (wParam) {
            case VK_SHIFT:
                key = KEY_CODE_MAP[MapVirtualKey(scanCode, MAPVK_VSC_TO_VK_EX)];
                break;

            case VK_CONTROL:
                key = extended ? KeyCode::RightCtrl : KeyCode::LeftCtrl;
                break;

            case VK_MENU:
                key = extended ? KeyCode::RightAlt : KeyCode::LeftAlt;
                break;

            default:
                key = KEY_CODE_MAP[wParam];
                break;
            }

            window->m_keyCallback(*window, key, action, mods);
        }

        return 0;
    }

    case WM_LBUTTONDOWN:
        if (window->m_keyCallback)
            window->m_keyCallback(*window, KeyCode::MouseLeft, KeyAction::Press, modifiers());
        return 0;

    case WM_LBUTTONUP:
        if (window->m_keyCallback)
            window->m_keyCallback(*window, KeyCode::MouseLeft, KeyAction::Release, modifiers());
        return 0;

    case WM_RBUTTONDOWN:
        if (window->m_keyCallback)
            window->m_keyCallback(*window, KeyCode::MouseRight, KeyAction::Press, modifiers());
        return 0;

    case WM_RBUTTONUP:
        if (window->m_keyCallback)
            window->m_keyCallback(*window, KeyCode::MouseRight, KeyAction::Release, modifiers());
        return 0;

    case WM_MBUTTONDOWN:
        if (window->m_keyCallback)
            window->m_keyCallback(*window, KeyCode::MouseMiddle, KeyAction::Press, modifiers());
        return 0;

    case WM_MBUTTONUP:
        if (window->m_keyCallback)
            window->m_keyCallback(*window, KeyCode::MouseMiddle, KeyAction::Release, modifiers());
        return 0;

    case WM_XBUTTONDOWN:
        if (window->m_keyCallback) {
            const KeyCode key =
                (GET_XBUTTON_WPARAM(wParam) == XBUTTON1) ? KeyCode::MouseX1 : KeyCode::MouseX2;
            window->m_keyCallback(*window, key, KeyAction::Press, modifiers());
        }
        return TRUE;

    case WM_XBUTTONUP:
        if (window->m_keyCallback) {
            const KeyCode key =
                (GET_XBUTTON_WPARAM(wParam) == XBUTTON1) ? KeyCode::MouseX1 : KeyCode::MouseX2;
            window->m_keyCallback(*window, key, KeyAction::Release, modifiers());
        }
        return TRUE;

    case WM_MOUSEMOVE:
        if (window->m_mousePosCallback) {
            const std::int32_t x = GET_X_LPARAM(lParam);
            const std::int32_t y = GET_Y_LPARAM(lParam);

            window->m_mousePosCallback(*window, x, y);
        }
        return 0;

    case WM_MOUSEWHEEL:
        if (window->m_mouseWheelCallback) {
            const std::int32_t delta = GET_WHEEL_DELTA_WPARAM(wParam);
            window->m_mouseWheelCallback(
                *window, 0, static_cast<float>(delta) / static_cast<float>(WHEEL_DELTA));
        }
        return 0;

    case WM_MOUSEHWHEEL:
        if (window->m_mouseWheelCallback) {
            const std::int32_t delta = GET_WHEEL_DELTA_WPARAM(wParam);
            window->m_mouseWheelCallback(
                *window, static_cast<float>(delta) / static_cast<float>(WHEEL_DELTA), 0);
        }
        return 0;

    case WM_SIZE: {
        if (wParam == SIZE_MINIMIZED) {
            if (window->m_minimizeCallback)
                window->m_minimizeCallback(*window);
            window->m_isMinimized = true;
        } else {
            // Restored from minimized state.
            if (window->m_isMinimized && window->m_restoreCallback)
                window->m_restoreCallback(*window);
            window->m_isMinimized = false;

            // Maximized.
            if (wParam == SIZE_MAXIMIZED && window->m_maximizeCallback)
                window->m_maximizeCallback(*window);
        }

        const std::uint32_t width  = LOWORD(lParam);
        const std::uint32_t height = HIWORD(lParam);

        if (window->m_resizeCallback)
            window->m_resizeCallback(*window, width, height);

        window->m_frameWidth  = width;
        window->m_frameHeight = height;

        RECT rect;
        GetWindowRect(hWnd, &rect);
        window->m_windowWidth  = static_cast<std::uint32_t>(rect.right - rect.left);
        window->m_windowHeight = static_cast<std::uint32_t>(rect.bottom - rect.top);

        return 0;
    }

    case WM_MOVE:
        if (window->m_windowPosCallback) {
            const std::int32_t x = GET_X_LPARAM(lParam);
            const std::int32_t y = GET_Y_LPARAM(lParam);

            window->m_windowPosCallback(*window, x, y);
        }
        return 0;

    case WM_DROPFILES:
        if (window->m_fileDropCallback) {
            auto drop = reinterpret_cast<HDROP>(wParam);

            const std::size_t fileCount = DragQueryFileW(drop, 0xFFFFFFFF, nullptr, 0);

            std::vector<std::string> paths;
            paths.reserve(fileCount);

            // Parse file paths.
            std::wstring tempPath;
            for (std::uint32_t i = 0; i < fileCount; ++i) {
                const std::uint32_t pathLength = DragQueryFileW(drop, i, nullptr, 0);
                tempPath.resize(pathLength);
                DragQueryFileW(drop, i, tempPath.data(), pathLength);
                paths.push_back(toUTF8String(tempPath));
            }

            window->m_fileDropCallback(*window, fileCount, paths.data());
            DragFinish(drop);
        }
        return 0;
    }

    return DefWindowProcW(hWnd, message, wParam, lParam);
}

ink::SwapChain::SwapChain(RenderDevice       &renderDevice,
                          IDXGIFactory6      *dxgiFactory,
                          ID3D12CommandQueue *cmdQueue,
                          HWND                window,
                          std::uint32_t       numBuffers,
                          DXGI_FORMAT         bufferFormat,
                          bool                enableTearing)
    : m_renderDevice(&renderDevice),
      m_swapChain(),
      m_isTearingEnabled(),
      m_bufferCount(numBuffers <= 2 ? 2 : 3),
      m_bufferIndex(),
      m_pixelFormat(bufferFormat),
      m_backBuffers(),
      m_presentFenceValues() {
    // Query tearing support.
    if (enableTearing) {
        BOOL tearingSupport = FALSE;
        if (SUCCEEDED(dxgiFactory->CheckFeatureSupport(DXGI_FEATURE_PRESENT_ALLOW_TEARING,
                                                       &tearingSupport, sizeof(tearingSupport))))
            m_isTearingEnabled = (tearingSupport == TRUE);
    }

    // Create swap chain.
    RECT rect;
    GetClientRect(window, &rect);

    UINT flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
    if (m_isTearingEnabled)
        flags |= DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING;

    const DXGI_SWAP_CHAIN_DESC1 desc{
        /* Width      = */ static_cast<UINT>(rect.right),
        /* Height     = */ static_cast<UINT>(rect.bottom),
        /* Format     = */ m_pixelFormat,
        /* Stereo     = */ FALSE,
        /* SampleDesc = */
        {
            /* Count   = */ 1,
            /* Quality = */ 0,
        },
        /* BufferUsage = */ DXGI_USAGE_RENDER_TARGET_OUTPUT | DXGI_USAGE_BACK_BUFFER,
        /* BufferCount = */ m_bufferCount,
        /* Scaling     = */ DXGI_SCALING_NONE,
        /* SwapEffect  = */ DXGI_SWAP_EFFECT_FLIP_DISCARD,
        /* AlphaMode   = */ DXGI_ALPHA_MODE_UNSPECIFIED,
        /* Flags       = */ flags,
    };

    // TODO: Add fullscreen support.

    HRESULT hr = dxgiFactory->CreateSwapChainForHwnd(cmdQueue, window, &desc, nullptr, nullptr,
                                                     m_swapChain.GetAddressOf());
    if (FAILED(hr))
        throw RenderAPIException(hr, "Failed to create swap chain.");

    // Disable Alt-Enter.
    dxgiFactory->MakeWindowAssociation(window, DXGI_MWA_NO_ALT_ENTER);

    // Get back buffers.
    for (std::uint32_t i = 0; i < m_bufferCount; ++i) {
        Microsoft::WRL::ComPtr<ID3D12Resource> backBuffer;
        hr = m_swapChain->GetBuffer(i, IID_PPV_ARGS(backBuffer.GetAddressOf()));
        if (FAILED(hr))
            throw RenderAPIException(hr, "Failed to get the " + std::to_string(i) +
                                             "'th back buffer from swap chain.");

        m_backBuffers[i].m_renderTargetView = renderDevice.newRenderTargetView();
        m_backBuffers[i].resetSwapChainBuffer(std::move(backBuffer));
    }
}

ink::SwapChain::SwapChain() noexcept
    : m_renderDevice(),
      m_swapChain(),
      m_isTearingEnabled(),
      m_bufferCount(),
      m_bufferIndex(),
      m_pixelFormat(DXGI_FORMAT_UNKNOWN),
      m_backBuffers(),
      m_presentFenceValues() {}

ink::SwapChain::SwapChain(SwapChain &&other) noexcept
    : m_renderDevice(other.m_renderDevice),
      m_swapChain(std::move(other.m_swapChain)),
      m_isTearingEnabled(other.m_isTearingEnabled),
      m_bufferCount(other.m_bufferCount),
      m_bufferIndex(other.m_bufferIndex),
      m_pixelFormat(other.m_pixelFormat),
      m_backBuffers(std::move(other.m_backBuffers)),
      m_presentFenceValues(other.m_presentFenceValues) {
    other.m_renderDevice     = nullptr;
    other.m_isTearingEnabled = false;
    other.m_bufferCount      = 0;
    other.m_bufferIndex      = 0;
    other.m_pixelFormat      = DXGI_FORMAT_UNKNOWN;
}

ink::SwapChain::~SwapChain() noexcept {
    if (m_swapChain != nullptr) {
        std::uint64_t maxFence =
            std::max({m_presentFenceValues[0], m_presentFenceValues[1], m_presentFenceValues[2]});
        m_renderDevice->sync(maxFence);

        // Change fullscreen state. Swap chain cannot be destroyed in fullscreen state.
        BOOL    isFullscreen = FALSE;
        HRESULT hr           = m_swapChain->GetFullscreenState(&isFullscreen, nullptr);
        if (SUCCEEDED(hr) && isFullscreen)
            m_swapChain->SetFullscreenState(FALSE, nullptr);
    }
}

auto ink::SwapChain::operator=(SwapChain &&other) noexcept -> SwapChain & {
    // Avoid self move.
    if (this == &other)
        return *this;

    // Destroy this swap chain.
    if (m_swapChain != nullptr) {
        std::uint64_t maxFence =
            std::max({m_presentFenceValues[0], m_presentFenceValues[1], m_presentFenceValues[2]});
        m_renderDevice->sync(maxFence);

        // Change fullscreen state. Swap chain cannot be destroyed in fullscreen state.
        BOOL    isFullscreen = FALSE;
        HRESULT hr           = m_swapChain->GetFullscreenState(&isFullscreen, nullptr);
        if (SUCCEEDED(hr) && isFullscreen)
            m_swapChain->SetFullscreenState(FALSE, nullptr);
    }

    m_renderDevice       = other.m_renderDevice;
    m_swapChain          = std::move(other.m_swapChain);
    m_isTearingEnabled   = other.m_isTearingEnabled;
    m_bufferCount        = other.m_bufferCount;
    m_bufferIndex        = other.m_bufferIndex;
    m_pixelFormat        = other.m_pixelFormat;
    m_backBuffers        = std::move(other.m_backBuffers);
    m_presentFenceValues = other.m_presentFenceValues;

    other.m_renderDevice     = nullptr;
    other.m_isTearingEnabled = false;
    other.m_bufferCount      = 0;
    other.m_bufferIndex      = 0;
    other.m_pixelFormat      = DXGI_FORMAT_UNKNOWN;

    return *this;
}

auto ink::SwapChain::present() noexcept -> void {
    m_swapChain->Present(0, m_isTearingEnabled ? DXGI_PRESENT_ALLOW_TEARING : 0);

    // Acquire a fence value.
    const std::uint64_t fenceValue      = m_renderDevice->signalFence();
    m_presentFenceValues[m_bufferIndex] = fenceValue;

    // Increase buffer index.
    m_bufferIndex = (m_bufferIndex + 1) % m_bufferCount;
}

auto ink::SwapChain::resize(WindowSize frameSize) -> void {
    { // Sync with GPU.
        std::uint64_t maxFence =
            std::max({m_presentFenceValues[0], m_presentFenceValues[1], m_presentFenceValues[2]});
        m_renderDevice->sync(maxFence);
    }

    // Release back buffers.
    for (std::uint32_t i = 0; i < m_bufferCount; ++i)
        m_backBuffers[i].releaseSwapChainResource();

    UINT flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
    if (m_isTearingEnabled)
        flags |= DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING;

    HRESULT hr = m_swapChain->ResizeBuffers(0, frameSize.width, frameSize.height,
                                            DXGI_FORMAT_UNKNOWN, flags);
    if (FAILED(hr))
        throw RenderAPIException(hr, "Failed to resize swap chain back buffers.");

    // Get back buffers.
    for (std::uint32_t i = 0; i < m_bufferCount; ++i) {
        Microsoft::WRL::ComPtr<ID3D12Resource> backBuffer;
        hr = m_swapChain->GetBuffer(i, IID_PPV_ARGS(backBuffer.GetAddressOf()));
        if (FAILED(hr))
            throw RenderAPIException(hr, "Failed to get the " + std::to_string(i) +
                                             "'th back buffer from swap chain.");

        m_backBuffers[i].resetSwapChainBuffer(std::move(backBuffer));
    }

    m_bufferIndex = 0;
}

auto ink::SwapChain::backBuffer() const noexcept -> ColorBuffer & {
    m_renderDevice->sync(m_presentFenceValues[m_bufferIndex]);
    return m_backBuffers[m_bufferIndex];
}

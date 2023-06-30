#include "ink/core/window.h"
#include "ink/core/assert.h"

#include <shellapi.h>
#include <windowsx.h>

using namespace ink;

namespace {

/// @brief
///   Window process function, for initialization usage only.
/// @remark
///   Initialization function is differ from normal window process function because some events may
///   be sent during window initialization which may call some window virtual callback functions
///   during construction.
///
/// @param hWnd
///   The win32 window handle to deal with the message.
/// @param uMsg
///   The message to handle.
/// @param wParam
///   wParam of the message.
/// @param lParam
///   lParam of the message.
///
/// @return
///   An integer that indicates the message handle result.
LRESULT CALLBACK windowInitializeProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) noexcept {
    if (uMsg == WM_NCCREATE) {
        auto cs = reinterpret_cast<CREATESTRUCTW *>(lParam);
        SetWindowLongPtrW(hWnd, 0, reinterpret_cast<LONG_PTR>(cs->lpCreateParams));
    }

    return DefWindowProcW(hWnd, uMsg, wParam, lParam);
}

class WindowClass {
private:
    /// @brief
    ///   Register ink default window class.
    WindowClass() noexcept : m_hInstance(GetModuleHandleW(nullptr)), m_classID() {
        const WNDCLASSEXW clsEx{
            /* cbSize        = */ sizeof(WNDCLASSEX),
            /* style         = */ CS_HREDRAW | CS_VREDRAW,
            /* lpfnWndProc   = */ windowInitializeProc,
            /* cbClsExtra    = */ 0,
            /* cbWndExtra    = */ sizeof(void *),
            /* hInstance     = */ m_hInstance,
            /* hIcon         = */ nullptr,
            /* hCursor       = */ LoadCursor(nullptr, IDC_ARROW),
            /* hbrBackground = */ reinterpret_cast<HBRUSH>(COLOR_WINDOW + 1),
            /* lpszMenuName  = */ nullptr,
            /* lpszClassName = */ L"ink",
            /* hIconSm       = */ nullptr,
        };

        m_classID = RegisterClassExW(&clsEx);
        inkAssert(m_classID != 0, u"Failed to register win32 window class: {}.", GetLastError());
    }

    /// @brief
    ///   Unregister ink default window class.
    ~WindowClass() noexcept {
        UnregisterClassW(reinterpret_cast<LPCWSTR>(m_classID), m_hInstance);
    }

public:
    /// @brief
    ///   Get default ink win32 window class.
    ///
    /// @return
    ///   Default ink win32 window class.
    [[nodiscard]]
    static auto get() noexcept -> LPCWSTR;

private:
    /// @brief
    ///   The HINSTANCE that is used to register this class.
    HINSTANCE m_hInstance;

    /// @brief
    ///   ID of this window class.
    ATOM m_classID;
};

auto WindowClass::get() noexcept -> LPCWSTR {
    static WindowClass instance;
    return reinterpret_cast<LPCWSTR>(instance.m_classID);
}

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

} // namespace

#if !defined(__clang__) && defined(_MSC_VER)
#    pragma warning(push)
#    pragma warning(disable : 6387)
#endif

ink::Window::Window(StringView    title,
                    std::uint32_t width,
                    std::uint32_t height,
                    WindowStyle   style) noexcept
    : m_hInstance(GetModuleHandleW(nullptr)),
      m_hWnd(nullptr),
      m_title(title),
      m_width(width),
      m_height(height),
      m_highSurrogate() {
    // Setup DPI awareness. It is safe to call for multi-times.
    SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);

    // Window class.
    LPCWSTR cls = WindowClass::get();

    DWORD dwStyle, dwStyleEx;
    toWin32Style(style, dwStyle, dwStyleEx);

    RECT rect{0, 0, static_cast<LONG>(width), static_cast<LONG>(height)};
    AdjustWindowRectEx(&rect, dwStyle, FALSE, dwStyleEx);

    // Create window.
    m_hWnd = CreateWindowExW(dwStyleEx, cls, reinterpret_cast<LPCWSTR>(m_title.data()), dwStyle,
                             CW_USEDEFAULT, CW_USEDEFAULT, rect.right - rect.left,
                             rect.bottom - rect.top, nullptr, nullptr, m_hInstance, this);
    inkAssert(m_hWnd != nullptr, u"Failed to create window {}: {}.", m_title, GetLastError());

    // Show window.
    if ((style & WindowStyle::Visible) != WindowStyle::None)
        ShowWindow(m_hWnd, SW_SHOW);

    // Change window process function.
    SetWindowLongPtrW(m_hWnd, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(Window::windowProc));
}

#if !defined(__clang__) && defined(_MSC_VER)
#    pragma warning(pop)
#endif

ink::Window::~Window() noexcept {
    if (m_hWnd != nullptr)
        DestroyWindow(m_hWnd);
}

auto ink::Window::resize(std::uint32_t width, std::uint32_t height) noexcept -> void {
    if (m_hWnd == nullptr)
        return;

    RECT rect{0, 0, static_cast<LONG>(width), static_cast<LONG>(height)};
    AdjustWindowRectEx(&rect, GetWindowLongW(m_hWnd, GWL_STYLE), FALSE,
                       GetWindowLongW(m_hWnd, GWL_EXSTYLE));

    SetWindowPos(m_hWnd, nullptr, 0, 0, rect.right - rect.left, rect.bottom - rect.top,
                 SWP_ASYNCWINDOWPOS | SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE);

    m_width  = width;
    m_height = height;
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

auto ink::Window::onFocus(bool focused) -> void {
    (void)focused;
}

auto ink::Window::onClose() -> void {}

auto ink::Window::onChar(char32_t codePoint) -> void {
    (void)codePoint;
}

auto ink::Window::onKey(KeyCode key, KeyAction action, Modifier mods) -> void {
    (void)key;
    (void)action;
    (void)mods;
}

auto ink::Window::onMouseMove(std::int32_t x, std::int32_t y) -> void {
    (void)x;
    (void)y;
}

auto ink::Window::onMouseWheel(
    std::int32_t x, std::int32_t y, float deltaX, float deltaY, Modifier mods) -> void {
    (void)x;
    (void)y;
    (void)deltaX;
    (void)deltaY;
    (void)mods;
}

auto ink::Window::onResize(std::uint32_t width, std::uint32_t height) -> void {
    (void)width;
    (void)height;
}

auto ink::Window::onMinimize() -> void {}

auto ink::Window::onMaximize() -> void {}

auto ink::Window::onMove(std::int32_t x, std::int32_t y) -> void {
    (void)x;
    (void)y;
}

auto ink::Window::onFileDrop(std::int32_t x, std::int32_t y, std::size_t count, String paths[])
    -> void {
    (void)x;
    (void)y;
    (void)count;
    (void)paths;
}

namespace {

/// @brief
///   Get modifier keys status.
///
/// @return
///   Modifier keys status.
[[nodiscard]]
auto modifiers() noexcept -> Modifier {
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
static constexpr const KeyCode KEY_CODE_MAP[0x100] = {
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

LRESULT ink::Window::windowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    Window *window = reinterpret_cast<Window *>(GetWindowLongPtrW(hWnd, 0));
    switch (uMsg) {
    case WM_NCCREATE: {
        auto cs = reinterpret_cast<CREATESTRUCTW *>(lParam);
        SetWindowLongPtrW(hWnd, 0, reinterpret_cast<LONG_PTR>(cs->lpCreateParams));
        return DefWindowProcW(hWnd, uMsg, wParam, lParam);
    }

    case WM_SETFOCUS:
        window->onFocus(true);
        return 0;

    case WM_KILLFOCUS:
        window->onFocus(false);
        return 0;

    case WM_CLOSE:
        window->onClose();
        DestroyWindow(hWnd);
        window->m_hWnd = nullptr;
        return 0;

    case WM_CHAR:
        [[fallthrough]];
    case WM_SYSCHAR:
        if (wParam >= 0xD800 && wParam <= 0xDBFF) {
            // High surrogate.
            window->m_highSurrogate = static_cast<WCHAR>(wParam);
        } else {
            char32_t codePoint = 0;
            if (wParam >= 0xDC00 && wParam <= 0xDFFF) {
                if (window->m_highSurrogate) {
                    codePoint += (window->m_highSurrogate - 0xD800) << 10;
                    codePoint += static_cast<WCHAR>(wParam) - 0xDC00;
                    codePoint += 0x10000;
                }
            } else {
                codePoint = static_cast<WCHAR>(wParam);
            }

            window->m_highSurrogate = 0;

            // Handle char callback.
            window->onChar(codePoint);
        }

        return 0;

    case WM_UNICHAR:
        if (wParam == UNICODE_NOCHAR)
            return TRUE;

        window->onChar(static_cast<char32_t>(wParam));
        return 0;

    case WM_KEYDOWN:
        [[fallthrough]];
    case WM_SYSKEYDOWN:
        [[fallthrough]];
    case WM_KEYUP:
        [[fallthrough]];
    case WM_SYSKEYUP: {
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

        window->onKey(key, action, mods);
        return 0;
    }

    case WM_LBUTTONDOWN:
        window->onKey(KeyCode::MouseLeft, KeyAction::Press, modifiers());
        return 0;

    case WM_LBUTTONUP:
        window->onKey(KeyCode::MouseLeft, KeyAction::Release, modifiers());
        return 0;

    case WM_RBUTTONDOWN:
        window->onKey(KeyCode::MouseRight, KeyAction::Press, modifiers());
        return 0;

    case WM_RBUTTONUP:
        window->onKey(KeyCode::MouseRight, KeyAction::Release, modifiers());
        return 0;

    case WM_MBUTTONDOWN:
        window->onKey(KeyCode::MouseMiddle, KeyAction::Press, modifiers());
        return 0;

    case WM_MBUTTONUP:
        window->onKey(KeyCode::MouseMiddle, KeyAction::Release, modifiers());
        return 0;

    case WM_XBUTTONDOWN: {
        const KeyCode key =
            (GET_XBUTTON_WPARAM(wParam) == XBUTTON1) ? KeyCode::MouseX1 : KeyCode::MouseX2;
        window->onKey(key, KeyAction::Press, modifiers());
        return TRUE;
    }

    case WM_XBUTTONUP: {
        const KeyCode key =
            (GET_XBUTTON_WPARAM(wParam) == XBUTTON1) ? KeyCode::MouseX1 : KeyCode::MouseX2;
        window->onKey(key, KeyAction::Release, modifiers());
        return TRUE;
    }

    case WM_MOUSEMOVE: {
        const std::int32_t x = GET_X_LPARAM(lParam);
        const std::int32_t y = GET_Y_LPARAM(lParam);

        window->onMouseMove(x, y);
        return 0;
    }

    case WM_MOUSEWHEEL: {
        const std::int32_t delta = GET_WHEEL_DELTA_WPARAM(wParam);
        const std::int32_t x     = GET_X_LPARAM(lParam);
        const std::int32_t y     = GET_Y_LPARAM(lParam);
        const auto         mods  = modifiers();

        window->onMouseWheel(x, y, 0, static_cast<float>(delta) / float(WHEEL_DELTA), mods);
        return 0;
    }

    case WM_MOUSEHWHEEL: {
        const std::int32_t delta = GET_WHEEL_DELTA_WPARAM(wParam);
        const std::int32_t x     = GET_X_LPARAM(lParam);
        const std::int32_t y     = GET_Y_LPARAM(lParam);
        const auto         mods  = modifiers();

        window->onMouseWheel(x, y, static_cast<float>(delta) / float(WHEEL_DELTA), 0, mods);
        return 0;
    }

    case WM_SIZE: {
        const std::uint32_t width  = LOWORD(lParam);
        const std::uint32_t height = HIWORD(lParam);

        if (wParam == SIZE_MINIMIZED)
            window->onMinimize();
        else if (wParam == SIZE_MAXIMIZED)
            window->onMaximize();

        window->onResize(width, height);
        return 0;
    }

    case WM_MOVE: {
        const std::int32_t x = GET_X_LPARAM(lParam);
        const std::int32_t y = GET_Y_LPARAM(lParam);

        window->onMove(x, y);
        return 0;
    }

    case WM_DROPFILES: {
        HDROP drop = reinterpret_cast<HDROP>(wParam);

        const std::size_t fileCount = DragQueryFileW(drop, 0xFFFFFFFF, nullptr, 0);

        std::vector<String> paths;
        paths.reserve(fileCount);

        // Query drop point.
        POINT point;
        DragQueryPoint(drop, &point);

        // Parse file paths.
        for (std::uint32_t i = 0; i < fileCount; ++i) {
            const std::uint32_t pathLength = DragQueryFileW(drop, i, nullptr, 0);

            String path;
            path.resize(pathLength);
            DragQueryFileW(drop, i, reinterpret_cast<LPWSTR>(path.data()), pathLength);

            paths.push_back(std::move(path));
        }

        window->onFileDrop(point.x, point.y, fileCount, paths.data());
        DragFinish(drop);

        return 0;
    }
    }

    return DefWindowProcW(hWnd, uMsg, wParam, lParam);
}

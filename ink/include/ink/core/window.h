#pragma once

#include "ink/core/string.h"

#include <Windows.h>

#include <functional>

namespace ink {

enum class KeyCode {
    Undefined = -1,

    // Mouse buttons
    MouseLeft   = 0x01,
    MouseRight  = 0x02,
    Break       = 0x03,
    MouseMiddle = 0x04,
    MouseX1     = 0x05,
    MouseX2     = 0x06,

    Backspace   = 0x08,
    Tab         = 0x09,
    Clear       = 0x0C,
    Enter       = 0x0D,
    Pause       = 0x13,
    CapsLock    = 0x14,
    Escape      = 0x1B,
    Space       = 0x20,
    PageUp      = 0x21,
    PageDown    = 0x22,
    End         = 0x23,
    Home        = 0x24,
    Left        = 0x25,
    Up          = 0x26,
    Right       = 0x27,
    Down        = 0x28,
    Select      = 0x29,
    Print       = 0x2A,
    Execute     = 0x2B,
    PrintScreen = 0x2C,
    Insert      = 0x2D,
    Delete      = 0x2E,
    Help        = 0x2F,

    // Digits
    Digit0 = 0x30,
    Digit1 = 0x31,
    Digit2 = 0x32,
    Digit3 = 0x33,
    Digit4 = 0x34,
    Digit5 = 0x35,
    Digit6 = 0x36,
    Digit7 = 0x37,
    Digit8 = 0x38,
    Digit9 = 0x39,

    // Alphabets
    A = 0x41,
    B = 0x42,
    C = 0x43,
    D = 0x44,
    E = 0x45,
    F = 0x46,
    G = 0x47,
    H = 0x48,
    I = 0x49,
    J = 0x4A,
    K = 0x4B,
    L = 0x4C,
    M = 0x4D,
    N = 0x4E,
    O = 0x4F,
    P = 0x50,
    Q = 0x51,
    R = 0x52,
    S = 0x53,
    T = 0x54,
    U = 0x55,
    V = 0x56,
    W = 0x57,
    X = 0x58,
    Y = 0x59,
    Z = 0x5A,

    LeftSuper   = 0x5B,
    RightSuper  = 0x5C,
    ContextMenu = 0x5D,
    Sleep       = 0x5F,

    // Keypad keys
    Keypad0  = 0x60,
    Keypad1  = 0x61,
    Keypad2  = 0x62,
    Keypad3  = 0x63,
    Keypad4  = 0x64,
    Keypad5  = 0x65,
    Keypad6  = 0x66,
    Keypad7  = 0x67,
    Keypad8  = 0x68,
    Keypad9  = 0x69,
    Multiply = 0x6A, // Keypad *
    Add      = 0x6B, // Keypad +
    Subtract = 0x6D, // Keypad -
    Decimal  = 0x6E, // Keypad .
    Divide   = 0x6F, // Keypad /

    F1  = 0x70,
    F2  = 0x71,
    F3  = 0x72,
    F4  = 0x73,
    F5  = 0x74,
    F6  = 0x75,
    F7  = 0x76,
    F8  = 0x77,
    F9  = 0x78,
    F10 = 0x79,
    F11 = 0x7A,
    F12 = 0x7B,
    F13 = 0x7C,
    F14 = 0x7D,
    F15 = 0x7E,
    F16 = 0x7F,
    F17 = 0x80,
    F18 = 0x81,
    F19 = 0x82,
    F20 = 0x83,
    F21 = 0x84,
    F22 = 0x85,
    F23 = 0x86,
    F24 = 0x87,

    NumLock    = 0x90,
    ScrollLock = 0x91,
    LeftShift  = 0xA0,
    RightShift = 0xA1,
    LeftCtrl   = 0xA2,
    RightCtrl  = 0xA3,
    LeftAlt    = 0xA4,
    RightAlt   = 0xA5,

    Semicolon    = 0xBA, // ; :
    Equal        = 0xBB, // = +
    Comma        = 0xBC, // , <
    Minus        = 0xBD, // - _
    Period       = 0xBE, // . >
    Slash        = 0xBF, // / ?
    BackQuote    = 0xC0, // ` ~
    LeftBracket  = 0xDB, // [ {
    BackSlash    = 0xDC, // \ |
    RightBracket = 0xDD, // ] }
    Quote        = 0xDE, // ' "
};

enum class KeyAction {
    Press,
    Release,
    Repeat,
};

enum class Modifier {
    None  = 0,
    Shift = (1 << 0),
    Ctrl  = (1 << 1),
    Alt   = (1 << 2),
    Super = (1 << 3),
};

constexpr auto operator~(Modifier a) noexcept -> Modifier {
    return static_cast<Modifier>(~static_cast<int>(a));
}

constexpr auto operator|(Modifier a, Modifier b) noexcept -> Modifier {
    return static_cast<Modifier>(static_cast<int>(a) | static_cast<int>(b));
}

constexpr auto operator&(Modifier a, Modifier b) noexcept -> Modifier {
    return static_cast<Modifier>(static_cast<int>(a) & static_cast<int>(b));
}

constexpr auto operator^(Modifier a, Modifier b) noexcept -> Modifier {
    return static_cast<Modifier>(static_cast<int>(a) ^ static_cast<int>(b));
}

constexpr auto operator|=(Modifier &a, Modifier b) noexcept -> Modifier & {
    return (a = (a | b));
}

constexpr auto operator&=(Modifier &a, Modifier b) noexcept -> Modifier & {
    return (a = (a & b));
}

constexpr auto operator^=(Modifier &a, Modifier b) noexcept -> Modifier & {
    return (a = (a ^ b));
}

/// @brief
///   Checks if the specified key code or mouse button is currently pressed.
///
/// @param key
///   The key code or mouse button to be checked.
///
/// @return
///   A boolean value that indicates whether the specified key or mouse button is pressed.
/// @retval true
///   The specified key or mouse button is currently pressed.
/// @retval false
///   The specified key or mouse button is currently not pressed.
[[nodiscard]]
inline auto isKeyPressed(KeyCode key) noexcept -> bool {
    return (::GetAsyncKeyState(static_cast<int>(key)) & 0x8000) != 0;
}

enum class WindowStyle {
    None      = 0,
    Titled    = (1 << 0),
    Resizable = (1 << 1),
    Visible   = (1 << 2),
    Topmost   = (1 << 3),

    Default = (Titled | Visible),
};

constexpr auto operator~(WindowStyle a) noexcept -> WindowStyle {
    return static_cast<WindowStyle>(~static_cast<int>(a));
}

constexpr auto operator|(WindowStyle a, WindowStyle b) noexcept -> WindowStyle {
    return static_cast<WindowStyle>(static_cast<int>(a) | static_cast<int>(b));
}

constexpr auto operator&(WindowStyle a, WindowStyle b) noexcept -> WindowStyle {
    return static_cast<WindowStyle>(static_cast<int>(a) & static_cast<int>(b));
}

constexpr auto operator^(WindowStyle a, WindowStyle b) noexcept -> WindowStyle {
    return static_cast<WindowStyle>(static_cast<int>(a) ^ static_cast<int>(b));
}

constexpr auto operator|=(WindowStyle &a, WindowStyle b) noexcept -> WindowStyle & {
    return (a = (a | b));
}

constexpr auto operator&=(WindowStyle &a, WindowStyle b) noexcept -> WindowStyle & {
    return (a = (a & b));
}

constexpr auto operator^=(WindowStyle &a, WindowStyle b) noexcept -> WindowStyle & {
    return (a = (a ^ b));
}

class Window {
public:
    /// @brief
    ///   Create a new window.
    /// @note
    ///   Errors are handled with assertion.
    ///
    /// @param title
    ///   Title of this window.
    /// @param width
    ///   Expected client width of this window.
    /// @param height
    ///   Expected client height of this window.
    /// @param style
    ///   Style flags of this window. Default window style is titled and visible.
    InkApi Window(StringView    title,
                  std::uint32_t width,
                  std::uint32_t height,
                  WindowStyle   style = WindowStyle::Default) noexcept;

    /// @brief
    ///   Copy constructor of window is disabled.
    Window(const Window &) = delete;

    /// @brief
    ///   Copy assignment of window is disabled.
    auto operator=(const Window &) = delete;

    /// @brief
    ///   Destroy this window.
    InkApi virtual ~Window() noexcept;

    /// @brief
    ///   Get title of this window.
    ///
    /// @return
    ///   Title of this window.
    [[nodiscard]]
    auto title() const noexcept -> StringView {
        return m_title;
    }

    /// @brief
    ///   Set a new title for this window.
    ///
    /// @param newTitle
    ///   New title to be set.
    auto title(StringView newTitle) noexcept -> void {
        m_title = newTitle;
        SetWindowTextW(m_hWnd, reinterpret_cast<LPCWSTR>(m_title.data()));
    }

    /// @brief
    ///   Get client width of this window.
    ///
    /// @return
    ///   Client width of this window.
    [[nodiscard]]
    auto width() const noexcept -> std::uint32_t {
        return m_width;
    }

    /// @brief
    ///   Get client height of this window.
    ///
    /// @return
    ///   Client height of this window.
    [[nodiscard]]
    auto height() const noexcept -> std::uint32_t {
        return m_height;
    }

    /// @brief
    ///   Resize client area for this window.
    ///
    /// @param width
    ///   New client width.
    /// @param height
    ///   New client height.
    InkApi auto resize(std::uint32_t width, std::uint32_t height) noexcept -> void;

    /// @brief
    ///   Move this window to center of this monitor.
    InkApi auto center() noexcept -> void;

    /// @brief
    ///   Show this window if hidden.
    auto show() noexcept -> void {
        ::ShowWindow(m_hWnd, SW_SHOW);
    }

    /// @brief
    ///   Hide this window if shown.
    auto hide() noexcept -> void {
        ::ShowWindow(m_hWnd, SW_HIDE);
    }

    /// @brief
    ///   Checks if this window has been closed.
    ///
    /// @return
    ///   A boolean that indicates whether this window has been closed.
    /// @retval true
    ///   This window has been closed.
    /// @retval false
    ///   This window is not closed.
    [[nodiscard]]
    auto isClosed() const noexcept -> bool {
        return m_hWnd == nullptr;
    }

    /// @brief
    ///   Post a close message to this window. This method returns immediately and will not wait
    ///   until the message to be handled.
    /// @note
    ///   This method posts a close request to this window and return immediately. What would happen
    ///   depends on how the window handles the close message. The default message handler would
    ///   destroy the window after the close callback.
    auto close() noexcept -> void {
        if (m_hWnd)
            PostMessageW(m_hWnd, WM_CLOSE, 0, 0);
    }

    /// @brief
    ///   Set window focus callback.
    ///
    /// @tparam Func
    ///   Type of the window focus callback. Should be like auto()(bool focused) -> void.
    ///
    /// @param callback
    ///   The focus callback function.
    template <typename Func, typename = std::enable_if_t<std::is_invocable_r_v<void, Func, bool>>>
    auto setFocusCallback(Func &&callback) {
        m_focusCallback = std::forward<Func>(callback);
    }

    /// @brief
    ///   Set window close callback.
    ///
    /// @tparam Func
    ///   Type of the window focus callback. Should be like auto()(void) -> void.
    ///
    /// @param callback
    ///   The close callback function.
    template <typename Func, typename = std::enable_if_t<std::is_invocable_r_v<void, Func>>>
    auto setCloseCallback(Func &&callback) -> void {
        m_closeCallback = std::forward<Func>(callback);
    }

    /// @brief
    ///   Set window character input callback.
    ///
    /// @tparam Func
    ///   Type of the window character input callback. Should be like auto()(char32_t codePoint) ->
    ///   void.
    ///
    /// @param callback
    ///   The window character input callback function.
    template <typename Func,
              typename = std::enable_if_t<std::is_invocable_r_v<void, Func, char32_t>>>
    auto setCharCallback(Func &&callback) -> void {
        m_charCallback = std::forward<Func>(callback);
    }

    /// @brief
    ///   Set window key event callback.
    ///
    /// @tparam Func
    ///   Type of the window key event callback. Should be like auto()(KeyCode key, KeyAction
    ///   action, Modifier mods) -> void.
    ///
    /// @param callback
    ///   The window key event callback function.
    template <typename Func,
              typename =
                  std::enable_if_t<std::is_invocable_r_v<void, Func, KeyCode, KeyAction, Modifier>>>
    auto setKeyCallback(Func &&callback) -> void {
        m_keyCallback = std::forward<Func>(callback);
    }

    /// @brief
    ///   Set mouse position callback.
    ///
    /// @tparam Func
    ///   Type of the mouse position callback. Should be like auto()(std::int32_t x, std::int32_t y)
    ///   -> void.
    ///
    /// @param callback
    ///   The mouse position callback function.
    template <
        typename Func,
        typename = std::enable_if_t<std::is_invocable_r_v<void, Func, std::int32_t, std::int32_t>>>
    auto setMousePositionCallback(Func &&callback) -> void {
        m_mousePosCallback = std::forward<Func>(callback);
    }

    /// @brief
    ///   Set mouse wheel callback.
    ///
    /// @tparam Func
    ///   Type of the mouse wheel callback. Should be like auto()(std::int32_t x, std::int32_t y,
    ///   float deltaX, float deltaY, Modifier mods) -> void.
    ///
    /// @param callback
    ///   The mouse wheel callback function.
    template <
        typename Func,
        typename = std::enable_if_t<
            std::is_invocable_r_v<void, Func, std::int32_t, std::int32_t, float, float, Modifier>>>
    auto setMouseWheelCallback(Func &&func) -> void {
        m_mouseWheelCallback = std::forward<Func>(func);
    }

    /// @brief
    ///   Set frame resize callback.
    ///
    /// @tparam Func
    ///   Type of the frame resize callback. Should be like auto()(std::uint32_t width,
    ///   std::uint32_t height) -> void.
    ///
    /// @param callback
    ///   The frame resize callback function.
    template <typename Func,
              typename =
                  std::enable_if_t<std::is_invocable_r_v<void, Func, std::uint32_t, std::uint32_t>>>
    auto setFrameResizeCallback(Func &&func) -> void {
        m_resizeCallback = std::forward<Func>(func);
    }

    /// @brief
    ///   Set window minimize callback.
    ///
    /// @tparam Func
    ///   Type of the window minimize callback. Should be like auto()() -> void.
    ///
    /// @param callback
    ///   The window minimize callback function.
    template <typename Func, typename = std::enable_if_t<std::is_invocable_r_v<void, Func>>>
    auto setMinimizeCallback(Func &&func) -> void {
        m_minimizeCallback = std::forward<Func>(func);
    }

    /// @brief
    ///   Set window maximize callback.
    ///
    /// @tparam Func
    ///   Type of the window maximize callback. Should be like auto()() -> void.
    ///
    /// @param callback
    ///   The window maximize callback function.
    template <typename Func, typename = std::enable_if_t<std::is_invocable_r_v<void, Func>>>
    auto setMaximizeCallback(Func &&func) -> void {
        m_maximizeCallback = std::forward<Func>(func);
    }

    /// @brief
    ///   Set window position callback.
    ///
    /// @tparam Func
    ///   Type of the window position callback. Should be like auto()(std::int32_t x, std::int32_t
    ///   y) -> void.
    ///
    /// @param callback
    ///   The window position callback function.
    template <
        typename Func,
        typename = std::enable_if_t<std::is_invocable_r_v<void, Func, std::int32_t, std::int32_t>>>
    auto setWindowPositionCallback(Func &&func) -> void {
        m_windowPosCallback = std::forward<Func>(func);
    }

    /// @brief
    ///   Set window file drop callback.
    ///
    /// @tparam Func
    ///   Type of the window file drop callback. Should be like auto()(std::int32_t x, std::int32_t
    ///   y, std::size_t count, String paths[]) -> void.
    ///
    /// @param callback
    ///   The window file drop callback function.
    template <
        typename Func,
        typename = std::enable_if_t<
            std::is_invocable_r_v<void, Func, std::int32_t, std::int32_t, std::size_t, String[]>>>
    auto setFileDropCallback(Func &&func) -> void {
        m_fileDropCallback = std::forward<Func>(func);
    }

    friend class SwapChain;

    /// @brief
    ///   Window message handler function.
    InkApi static LRESULT CALLBACK windowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

protected:
    /// @brief
    ///   Get win32 native window handle of this window.
    ///
    /// @return
    ///   Win32 native window handle of this window.
    [[nodiscard]]
    auto nativeHandle() const noexcept -> HWND {
        return m_hWnd;
    }

private:
    /// @brief
    ///   The module that is used to create this window.
    HINSTANCE m_hInstance;

    /// @brief
    ///   Win32 window handle.
    HWND m_hWnd;

    /// @brief
    ///   Title of this window.
    String m_title;

    /// @brief
    ///   Client width of this window.
    std::uint32_t m_width;

    /// @brief
    ///   Client height of this window.
    std::uint32_t m_height;

    /// @brief
    ///   For internal usage. High surrogate for char message.
    WCHAR m_highSurrogate;

    /// @brief
    ///   Window focus callback.
    std::function<void(bool)> m_focusCallback;

    /// @brief
    ///   Window close callback.
    std::function<void()> m_closeCallback;

    /// @brief
    ///   Window character input callback.
    std::function<void(char32_t)> m_charCallback;

    /// @brief
    ///   Window key and mouse event callback.
    std::function<void(KeyCode, KeyAction, Modifier)> m_keyCallback;

    /// @brief
    ///   Mouse position callback.
    std::function<void(std::int32_t, std::int32_t)> m_mousePosCallback;

    /// @brief
    ///   Mouse wheel callback.
    std::function<void(std::int32_t, std::int32_t, float, float, Modifier)> m_mouseWheelCallback;

    /// @brief
    ///   Window client resize callback.
    std::function<void(std::uint32_t, std::uint32_t)> m_resizeCallback;

    /// @brief
    ///   Window minimize callback.
    std::function<void()> m_minimizeCallback;

    /// @brief
    ///   Window maximize callback.
    std::function<void()> m_maximizeCallback;

    /// @brief
    ///   Window position callback.
    std::function<void(std::int32_t, std::int32_t)> m_windowPosCallback;

    /// @brief
    ///   Window file drop callback.
    std::function<void(std::int32_t, std::int32_t, std::size_t, String[])> m_fileDropCallback;
};

} // namespace ink

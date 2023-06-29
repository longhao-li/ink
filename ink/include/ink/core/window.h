#pragma once

#include "ink/core/string.h"

#include <Windows.h>

namespace ink {

enum class KeyCode {
    Undefined = -1,

    // Mouse buttons
    MouseLeft   = VK_LBUTTON,
    MouseRight  = VK_RBUTTON,
    Break       = VK_CANCEL,
    MouseMiddle = VK_MBUTTON,
    MouseX1     = VK_XBUTTON1,
    MouseX2     = VK_XBUTTON2,

    Backspace   = VK_BACK,
    Tab         = VK_TAB,
    Clear       = VK_CLEAR,
    Enter       = VK_RETURN,
    Pause       = VK_PAUSE,
    CapsLock    = VK_CAPITAL,
    Escape      = VK_ESCAPE,
    Space       = VK_SPACE,
    PageUp      = VK_PRIOR,
    PageDown    = VK_NEXT,
    End         = VK_END,
    Home        = VK_HOME,
    Left        = VK_LEFT,
    Up          = VK_UP,
    Right       = VK_RIGHT,
    Down        = VK_DOWN,
    Select      = VK_SELECT,
    Print       = VK_PRINT,
    Execute     = VK_EXECUTE,
    PrintScreen = VK_SNAPSHOT,
    Insert      = VK_INSERT,
    Delete      = VK_DELETE,
    Help        = VK_HELP,

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

    LeftSuper   = VK_LWIN,
    RightSuper  = VK_RWIN,
    ContextMenu = VK_APPS,
    Sleep       = VK_SLEEP,

    // Keypad keys
    Keypad0  = VK_NUMPAD0,
    Keypad1  = VK_NUMPAD1,
    Keypad2  = VK_NUMPAD2,
    Keypad3  = VK_NUMPAD3,
    Keypad4  = VK_NUMPAD4,
    Keypad5  = VK_NUMPAD5,
    Keypad6  = VK_NUMPAD6,
    Keypad7  = VK_NUMPAD7,
    Keypad8  = VK_NUMPAD8,
    Keypad9  = VK_NUMPAD9,
    Multiply = VK_MULTIPLY, // Keypad *
    Add      = VK_ADD,      // Keypad +
    Subtract = VK_SUBTRACT, // Keypad -
    Decimal  = VK_DECIMAL,  // Keypad .
    Divide   = VK_DIVIDE,   // Keypad /

    F1  = VK_F1,
    F2  = VK_F2,
    F3  = VK_F3,
    F4  = VK_F4,
    F5  = VK_F5,
    F6  = VK_F6,
    F7  = VK_F7,
    F8  = VK_F8,
    F9  = VK_F9,
    F10 = VK_F10,
    F11 = VK_F11,
    F12 = VK_F12,
    F13 = VK_F13,
    F14 = VK_F14,
    F15 = VK_F15,
    F16 = VK_F16,
    F17 = VK_F17,
    F18 = VK_F18,
    F19 = VK_F19,
    F20 = VK_F20,
    F21 = VK_F21,
    F22 = VK_F22,
    F23 = VK_F23,
    F24 = VK_F24,

    NumLock    = VK_NUMLOCK,
    ScrollLock = VK_SCROLL,
    LeftShift  = VK_LSHIFT,
    RightShift = VK_RSHIFT,
    LeftCtrl   = VK_LCONTROL,
    RightCtrl  = VK_RCONTROL,
    LeftAlt    = VK_LMENU,
    RightAlt   = VK_RMENU,

    Semicolon    = VK_OEM_1,      // ; :
    Equal        = VK_OEM_PLUS,   // = +
    Comma        = VK_OEM_COMMA,  // , <
    Minus        = VK_OEM_MINUS,  // - _
    Period       = VK_OEM_PERIOD, // . >
    Slash        = VK_OEM_2,      // / ?
    BackQuote    = VK_OEM_3,      // ` ~
    LeftBracket  = VK_OEM_4,      // [ {
    BackSlash    = VK_OEM_5,      // \ |
    RightBracket = VK_OEM_6,      // ] }
    Quote        = VK_OEM_7,      // ' "
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
    Window(StringView    title,
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
    virtual ~Window() noexcept;

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
    auto resize(std::uint32_t width, std::uint32_t height) noexcept -> void;

    /// @brief
    ///   Move this window to center of this monitor.
    auto center() noexcept -> void;

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
    ///   Focus change callback for this window. This method is called when the window focus status
    ///   is changed.
    ///
    /// @param focused
    ///   Specifies whether this window gained or lost focus.
    virtual auto onFocus(bool focused) -> void;

    /// @brief
    ///   Close callback for this window. This method is called before this window is closed. This
    ///   callback may be used to release resources that should be freed before window destroy.
    virtual auto onClose() -> void;

    /// @brief
    ///   Character callback for this window. This method is called when this window receives a
    ///   character.
    ///
    /// @param codePoint
    ///   The character unicode code point.
    virtual auto onChar(char32_t codePoint) -> void;

    /// @brief
    ///   Key and mouse button callback for this window. This method is called when keyboard key or
    ///   mouse button status are changed.
    ///
    /// @param key
    ///   The key whose status is changed.
    /// @param action
    ///   Specifies key action of the specified key.
    /// @param mods
    ///   Modifier keys that are pressed when the key status is changed.
    virtual auto onKey(KeyCode key, KeyAction action, Modifier mods) -> void;

    /// @brief
    ///   Mouse move callback for this window. This callback is called when mouse is moved.
    ///
    /// @param x
    ///   New x position of mouse.
    /// @param y
    ///   New y position of mouse.
    virtual auto onMouseMove(std::int32_t x, std::int32_t y) -> void;

    /// @brief
    ///   Mouse wheel callback for this window. This callback is called when mouse wheels.
    ///
    /// @param x
    ///   X position of cursor.
    /// @param y
    ///   Y position of cursor.
    /// @param deltaX
    ///   Mouse wheel delta x.
    /// @param deltaY
    ///   Mouse wheel delta y.
    /// @param mods
    ///   Modifier keys that are pressed when mouse is wheeling.
    virtual auto
    onMouseWheel(std::int32_t x, std::int32_t y, float deltaX, float deltaY, Modifier mods) -> void;

    /// @brief
    ///   Window resize callback for this window. This callback is called if window size is changed.
    ///
    /// @param width
    ///   New client width of this window.
    /// @param height
    ///   New client height of this window.
    virtual auto onResize(std::uint32_t width, std::uint32_t height) -> void;

    /// @brief 
    ///   Window minimize callback for this window. This callback is called if window is minimized.
    virtual auto onMinimize() -> void;

    /// @brief 
    ///   Window maximize callback for this window. This callback is called if window is maximized.
    virtual auto onMaximize() -> void;

    /// @brief
    ///   Window move callback for this window. This callback is called if window position is
    ///   changed.
    ///
    /// @param x
    ///   New window x coordinate.
    /// @param y
    ///   New window y coordinate.
    virtual auto onMove(std::int32_t x, std::int32_t y) -> void;

    /// @brief
    ///   File drop callback for this window. This callback is called if user try to drop files to
    ///   this window.
    ///
    /// @param x
    ///   X coordinate of the cursor.
    /// @param y
    ///   Y coordinate of the cursor.
    /// @param count
    ///   Number of files dropped.
    /// @param paths
    ///   Paths of the files dropped to this window.
    virtual auto onFileDrop(std::int32_t x, std::int32_t y, std::size_t count, String paths[])
        -> void;

    friend class SwapChain;

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

    /// @brief
    ///   Window message handler function.
    static LRESULT CALLBACK windowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

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
};

} // namespace ink

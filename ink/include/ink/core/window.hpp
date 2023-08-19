#pragma once

#include "../render/resource.hpp"

#include <dxgi1_6.h>

#include <array>
#include <functional>
#include <string>

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

constexpr auto operator|=(Modifier &a, Modifier b) noexcept -> Modifier & { return (a = (a | b)); }

constexpr auto operator&=(Modifier &a, Modifier b) noexcept -> Modifier & { return (a = (a & b)); }

constexpr auto operator^=(Modifier &a, Modifier b) noexcept -> Modifier & { return (a = (a ^ b)); }

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
[[nodiscard]] inline auto isKeyPressed(KeyCode key) noexcept -> bool {
    return (::GetAsyncKeyState(static_cast<int>(key)) & 0x8000) != 0;
}

enum class WindowStyle {
    None      = 0,
    Titled    = (1 << 0),
    Resizable = (1 << 1),
    Topmost   = (1 << 2),
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

struct WindowSize {
    std::uint32_t width;
    std::uint32_t height;
};

class Window {
public:
    using FocusFunc      = std::function<void(Window &, bool)>;
    using CloseFunc      = std::function<void(Window &)>;
    using CharFunc       = std::function<void(Window &, char32_t)>;
    using KeyFunc        = std::function<void(Window &, KeyCode, KeyAction, Modifier)>;
    using MousePosFunc   = std::function<void(Window &, std::int32_t, std::int32_t)>;
    using MouseWheelFunc = std::function<void(Window &, float, float)>;
    using ResizeFunc     = std::function<void(Window &, std::uint32_t, std::uint32_t)>;
    using MinimizeFunc   = std::function<void(Window &)>;
    using RestoreFunc    = std::function<void(Window &)>;
    using MaximizeFunc   = std::function<void(Window &)>;
    using WindowPosFunc  = std::function<void(Window &, std::int32_t, std::int32_t)>;
    using FileDropFunc   = std::function<void(Window &, std::size_t, std::string *)>;

    /// @brief
    ///   Create a new window.
    ///
    /// @param title
    ///   Title of this window. Should be encoded in UTF-8.
    /// @param width
    ///   Frame width in pixel of this window.
    /// @param height
    ///   Frame height in pixel of this window.
    /// @param style
    ///   Style of this window. Default window style is titled.
    ///
    /// @throw RenderAPIException
    InkExport Window(std::string_view title,
                     std::uint32_t    width,
                     std::uint32_t    height,
                     WindowStyle      style = WindowStyle::Titled);

    /// @brief
    ///   Copy constructor of window is disabled.
    Window(const Window &) = delete;

    /// @brief
    ///   Move constructor of window. Though it is supported, it is not recommended to use move
    ///   constructor for window, since it is likely that pointer to this window may be held by
    ///   other objects.
    ///
    /// @param other
    ///   The window to be moved. The moved window will be invalidated.
    InkExport Window(Window &&other) noexcept;

    /// @brief
    ///   Destroy this window. It is safe to inherit from this class.
    InkExport virtual ~Window() noexcept;

    /// @brief
    ///   Get title of this window. Title is encoded in UTF-8.
    ///
    /// @return
    ///   Title of this window.
    [[nodiscard]] auto title() const noexcept -> std::string_view { return m_title; }

    /// @brief
    ///   Set a new title for this window.
    ///
    /// @param newTitle
    ///   New title of this window. Should be encoded in UTF-8.
    InkExport auto title(std::string_view newTitle) noexcept -> void;

    /// @brief
    ///   Get size of this window. The window title bar and border are included and therefore this
    ///   is not the frame size.
    ///
    /// @return
    ///   Size of this window.
    [[nodiscard]] auto size() const noexcept -> WindowSize {
        return {m_windowWidth, m_windowHeight};
    }

    /// @brief
    ///   Get frame size of this window. This could be used to create a swap chain.
    ///
    /// @return
    ///   Frame size of this window.
    [[nodiscard]] auto frameSize() const noexcept -> WindowSize {
        return {m_frameWidth, m_frameHeight};
    }

    /// @brief
    ///   Resize frame of this window.
    ///
    /// @param frame
    ///   Expected new frame size of this window.
    InkExport auto resize(WindowSize frame) noexcept -> void;

    /// @brief
    ///   Move this window to the center of the screen.
    InkExport auto center() noexcept -> void;

    /// @brief
    ///   Show this window if hidden.
    auto show() noexcept -> void { ::ShowWindowAsync(m_hWnd, SW_SHOW); }

    /// @brief
    ///   Hide this window if shown.
    auto hide() noexcept -> void { ::ShowWindowAsync(m_hWnd, SW_HIDE); }

    /// @brief
    ///   Checks if this window has been closed.
    ///
    /// @return
    ///   A boolean that indicates whether this window has been closed.
    /// @retval true
    ///   This window has been closed.
    /// @retval false
    ///   This window is not closed.
    [[nodiscard]] auto isClosed() const noexcept -> bool { return m_hWnd == nullptr; }

    /// @brief
    ///   Post a close message to this window. This method returns immediately and will not wait
    ///   until the message to be handled.
    /// @note
    ///   This method posts a close request to this window and return immediately. What would happen
    ///   depends on how the window handles the close message. The default message handler would
    ///   destroy the window after the close callback.
    auto close() noexcept -> void {
        if (!isClosed())
            ::PostMessageW(m_hWnd, WM_CLOSE, 0, 0);
    }

    /// @brief
    ///   Set window focus callback.
    ///
    /// @tparam Func
    ///   Type of the window focus callback. Should be like auto()(Window &window, bool focused) ->
    ///   void.
    ///
    /// @param callback
    ///   The focus callback function.
    template <typename Func, typename = std::enable_if_t<std::is_assignable_v<FocusFunc, Func &&>>>
    auto setFocusCallback(Func &&callback) {
        m_focusCallback = std::forward<Func>(callback);
    }

    /// @brief
    ///   Set window close callback.
    ///
    /// @tparam Func
    ///   Type of the window focus callback. Should be like auto()(Window &window) -> void.
    ///
    /// @param callback
    ///   The close callback function.
    template <typename Func, typename = std::enable_if_t<std::is_assignable_v<CloseFunc, Func &&>>>
    auto setCloseCallback(Func &&callback) -> void {
        m_closeCallback = std::forward<Func>(callback);
    }

    /// @brief
    ///   Set window character input callback.
    ///
    /// @tparam Func
    ///   Type of the window character input callback. Should be like auto()(Window &window,
    ///   char32_t codePoint) -> void.
    ///
    /// @param callback
    ///   The window character input callback function.
    template <typename Func, typename = std::enable_if_t<std::is_assignable_v<CharFunc, Func &&>>>
    auto setCharCallback(Func &&callback) -> void {
        m_charCallback = std::forward<Func>(callback);
    }

    /// @brief
    ///   Set window key event callback.
    ///
    /// @tparam Func
    ///   Type of the window key event callback. Should be like auto()(Window &window, KeyCode key,
    ///   KeyAction action, Modifier mods) -> void.
    ///
    /// @param callback
    ///   The window key event callback function.
    template <typename Func, typename = std::enable_if_t<std::is_assignable_v<KeyFunc, Func &&>>>
    auto setKeyCallback(Func &&callback) -> void {
        m_keyCallback = std::forward<Func>(callback);
    }

    /// @brief
    ///   Set mouse position callback.
    ///
    /// @tparam Func
    ///   Type of the mouse position callback. Should be like auto()(Window &window, std::int32_t x,
    ///   std::int32_t y) -> void.
    ///
    /// @param callback
    ///   The mouse position callback function.
    template <typename Func,
              typename = std::enable_if_t<std::is_assignable_v<MousePosFunc, Func &&>>>
    auto setMousePositionCallback(Func &&callback) -> void {
        m_mousePosCallback = std::forward<Func>(callback);
    }

    /// @brief
    ///   Set mouse wheel callback.
    ///
    /// @tparam Func
    ///   Type of the mouse wheel callback. Should be like auto()(Window &window, std::int32_t x,
    ///   std::int32_t y, float deltaX, float deltaY, Modifier mods) -> void.
    ///
    /// @param callback
    ///   The mouse wheel callback function.
    template <typename Func,
              typename = std::enable_if_t<std::is_assignable_v<MouseWheelFunc, Func &&>>>
    auto setMouseWheelCallback(Func &&func) -> void {
        m_mouseWheelCallback = std::forward<Func>(func);
    }

    /// @brief
    ///   Set frame resize callback.
    ///
    /// @tparam Func
    ///   Type of the frame resize callback. Should be like auto()(Window &window, std::uint32_t
    ///   width, std::uint32_t height) -> void.
    ///
    /// @param callback
    ///   The frame resize callback function.
    template <typename Func, typename = std::enable_if_t<std::is_assignable_v<ResizeFunc, Func &&>>>
    auto setFrameResizeCallback(Func &&func) -> void {
        m_resizeCallback = std::forward<Func>(func);
    }

    /// @brief
    ///   Set window minimize callback.
    ///
    /// @tparam Func
    ///   Type of the window minimize callback. Should be like auto()(Window &window) -> void.
    ///
    /// @param callback
    ///   The window minimize callback function.
    template <typename Func,
              typename = std::enable_if_t<std::is_assignable_v<MinimizeFunc, Func &&>>>
    auto setMinimizeCallback(Func &&func) -> void {
        m_minimizeCallback = std::forward<Func>(func);
    }

    /// @brief
    ///   Set window restore callback.
    ///
    /// @tparam Func
    ///   Type of the window restore callback. Should be like auto()(Window &window) -> void.
    ///
    /// @param callback
    ///   The window restore callback function.
    template <typename Func,
              typename = std::enable_if_t<std::is_assignable_v<RestoreFunc, Func &&>>>
    auto setRestoreCallback(Func &&func) -> void {
        m_restoreCallback = std::forward<Func>(func);
    }

    /// @brief
    ///   Set window maximize callback.
    ///
    /// @tparam Func
    ///   Type of the window maximize callback. Should be like auto()(Window &window) -> void.
    ///
    /// @param callback
    ///   The window maximize callback function.
    template <typename Func,
              typename = std::enable_if_t<std::is_assignable_v<MaximizeFunc, Func &&>>>
    auto setMaximizeCallback(Func &&func) -> void {
        m_maximizeCallback = std::forward<Func>(func);
    }

    /// @brief
    ///   Set window position callback.
    ///
    /// @tparam Func
    ///   Type of the window position callback. Should be like auto()(Window &window, std::int32_t
    ///   x, std::int32_t y) -> void.
    ///
    /// @param callback
    ///   The window position callback function.
    template <typename Func,
              typename = std::enable_if_t<std::is_assignable_v<WindowPosFunc, Func &&>>>
    auto setWindowPositionCallback(Func &&func) -> void {
        m_windowPosCallback = std::forward<Func>(func);
    }

    /// @brief
    ///   Set window file drop callback.
    ///
    /// @tparam Func
    ///   Type of the window file drop callback. Should be like auto()(Window &window, std::int32_t
    ///   x, std::int32_t y, std::size_t count, std::string paths[]) -> void.
    ///
    /// @param callback
    ///   The window file drop callback function.
    template <typename Func,
              typename = std::enable_if_t<std::is_assignable_v<FileDropFunc, Func &&>>>
    auto setFileDropCallback(Func &&func) -> void {
        m_fileDropCallback = std::forward<Func>(func);
    }

    friend class RenderDevice;

protected:
    /// @brief
    ///   For internal usage. Win32 window procedure.
    InkExport static auto CALLBACK windowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
        -> LRESULT;

private:
    /// @brief
    ///   The HINSTANCE that is used to register the window class.
    HINSTANCE m_hInstance;

    /// @brief
    ///   The window class name.
    ATOM m_class;

    /// @brief
    ///   Win32 window handle.
    HWND m_hWnd;

    /// @brief
    ///   Title of this window. Encoded in UTF-8.
    std::string m_title;

    /// @brief
    ///   Width of this window.
    std::uint32_t m_windowWidth;

    /// @brief
    ///   Height of this window.
    std::uint32_t m_windowHeight;

    /// @brief
    ///   Width in pixel of the frame buffer.
    std::uint32_t m_frameWidth;

    /// @brief
    ///   Height in pixel of the frame buffer.
    std::uint32_t m_frameHeight;

    /// @brief
    ///   For internal usage. Specifies whether the window is currently minimized.
    bool m_isMinimized;

    /// @brief
    ///   For internal usage. High surrogate of the UTF-16 character for char message.
    WCHAR m_highSurrogate;

    /// @brief
    ///   Window focus callback.
    FocusFunc m_focusCallback;

    /// @brief
    ///   Window close callback.
    CloseFunc m_closeCallback;

    /// @brief
    ///   Window character input callback.
    CharFunc m_charCallback;

    /// @brief
    ///   Window key and mouse event callback.
    KeyFunc m_keyCallback;

    /// @brief
    ///   Mouse position callback.
    MousePosFunc m_mousePosCallback;

    /// @brief
    ///   Mouse wheel callback.
    MouseWheelFunc m_mouseWheelCallback;

    /// @brief
    ///   Window client resize callback.
    ResizeFunc m_resizeCallback;

    /// @brief
    ///   Window minimize callback.
    MinimizeFunc m_minimizeCallback;

    /// @brief
    ///   Window restore from minimize callback.
    RestoreFunc m_restoreCallback;

    /// @brief
    ///   Window maximize callback.
    MaximizeFunc m_maximizeCallback;

    /// @brief
    ///   Window position callback.
    WindowPosFunc m_windowPosCallback;

    /// @brief
    ///   Window file drop callback.
    FileDropFunc m_fileDropCallback;
};

class SwapChain {
private:
    /// @brief
    ///   For internal usage. Create a new swap chain for the specified window.
    ///
    /// @param renderDevice
    ///   The render device that is used to create the swap chain.
    /// @param dxgiFactory
    ///   The DXGI factory that is used to create this swap chain.
    /// @param cmdQueue
    ///   The command queue that this swap chain submits present commands to.
    /// @param window
    ///   The window that this swap chain is created for.
    /// @param numBuffer
    ///   Expected number of back buffers of this swap chain. This value will always be clamped
    ///   between 2 and 3.
    /// @param bufferFormat
    ///   Back buffer pixel format.
    /// @param enableTearing
    ///   Specifies whether to enable variable refresh rate for this swap chain.
    ///
    /// @throw RenderAPIException
    ///   Thrown if failed to create DXGI and D3D12 objects for swap chain.
    SwapChain(RenderDevice       &renderDevice,
              IDXGIFactory6      *dxgiFactory,
              ID3D12CommandQueue *cmdQueue,
              HWND                window,
              std::uint32_t       numBuffers,
              DXGI_FORMAT         bufferFormat,
              bool                enableTearing);

    friend class RenderDevice;

public:
    /// @brief
    ///   Create an empty swap chain.
    InkExport SwapChain() noexcept;

    /// @brief
    ///   Copy constructor for swap chain is disabled.
    SwapChain(const SwapChain &) = delete;

    /// @brief
    ///   Move constructor of swap chain. Though it is supported, it is not recommended to use move
    ///   constructor for swap chain, since it is likely that pointer to this swap chain may be held
    ///   by other objects.
    ///
    /// @param other
    ///   The swap chain to be moved. The moved swap chain will be invalidated.
    InkExport SwapChain(SwapChain &&other) noexcept;

    /// @brief
    ///   Destroy this swap chain.
    InkExport ~SwapChain() noexcept;

    /// @brief
    ///   Copy assignment for swap chain is disabled.
    auto operator=(const SwapChain &) = delete;

    /// @brief
    ///   Move assignment of swap chain. Though it is supported, it is not recommended to use move
    ///   assignment for swap chain, since it is likely that pointer to this swap chain may be held
    ///   by other objects.
    ///
    /// @param other
    ///   The swap chain to be moved. The moved swap chain will be invalidated.
    InkExport auto operator=(SwapChain &&other) noexcept -> SwapChain &;

    /// @brief
    ///   Present current back buffer.
    /// @note
    ///   This method returns immediately and will not block current thread.
    InkExport auto present() noexcept -> void;

    /// @brief
    ///   Resize back buffers of this swap chain. This method will perform a synchronization
    ///   operation between CPU and GPU to wait for all back buffers idle.
    ///
    /// @param frameSize
    ///   Expected new back buffer size. Pass (0, 0) to use the window frame size as new back buffer
    ///   size.
    ///
    /// @throw RenderAPIException
    ///   Thrown if failed to resize swap chain back buffers.
    InkExport auto resize(WindowSize frameSize) -> void;

    /// @brief
    ///   Checks if variable refresh rate is enabled for this swap chain.
    ///
    /// @return
    ///   A boolean value that specifies whether variable refresh rate is enabled for this swap
    ///   chain.
    /// @retval true
    ///   Variable refresh rate is enabled for this swap chain.
    /// @retval false
    ///   Variable refresh rate is not enabled for this swap chain.
    [[nodiscard]] auto isTearingEnabled() const noexcept -> bool { return m_isTearingEnabled; }

    /// @brief
    ///   Wait for current back buffer available and get current back buffer.
    /// @note
    ///   This method will block current thread until current back buffer is available.
    ///
    /// @return
    ///   Reference to current back buffer.
    [[nodiscard]] InkExport auto backBuffer() const noexcept -> ColorBuffer &;

    /// @brief
    ///   Get back buffer pixel format.
    ///
    /// @return
    ///   Back buffer pixel format.
    [[nodiscard]] auto pixelFormat() const noexcept -> DXGI_FORMAT { return m_pixelFormat; }

    /// @brief
    ///   Get total number of back buffers in this swap chain.
    ///
    /// @return
    ///   Number of back buffers in this swap chain.
    [[nodiscard]] auto bufferCount() const noexcept -> std::uint32_t { return m_bufferCount; }

    /// @brief
    ///   Set clear color for all back buffers.
    ///
    /// @param color
    ///   New clear color to be set for back buffers.
    auto setClearColor(const Color &color) noexcept -> void {
        for (auto &backBuffer : m_backBuffers)
            backBuffer.clearColor(color);
    }

private:
    /// @brief
    ///   The render device that is used to create this swap chain.
    RenderDevice *m_renderDevice;

    /// @brief
    ///   DXGI swap chain object.
    Microsoft::WRL::ComPtr<IDXGISwapChain1> m_swapChain;

    /// @brief
    ///   Specifies whether tearing is enabled.
    bool m_isTearingEnabled;

    /// @brief
    ///   Total number of back buffers in this swap chain. Must either be 2 or 3.
    std::uint32_t m_bufferCount;

    /// @brief
    ///   Index of current back buffer.
    std::uint32_t m_bufferIndex;

    /// @brief
    ///   Back buffer pixel format.
    DXGI_FORMAT m_pixelFormat;

    /// @brief
    ///   Back buffers of this swap chain.
    mutable std::array<ColorBuffer, 3> m_backBuffers;

    /// @brief
    ///   Fence values that indicates when the corresponding back buffer could be reused.
    std::array<std::uint64_t, 3> m_presentFenceValues;
};

} // namespace ink

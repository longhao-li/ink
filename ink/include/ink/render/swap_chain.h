#pragma once

#include "ink/render/device.h"
#include "ink/render/resource.h"

#include <dxgi1_6.h>

namespace ink {

class SwapChain {
public:
    /// @brief
    ///   Create an empty swap chain object.
    InkApi SwapChain() noexcept;

    /// @brief
    ///   Create a new swap chain for the specified window.
    /// @note
    ///   Errors are handled with assertions.
    ///
    /// @param window
    ///   The window that this swap chain is created for.
    /// @param numBuffers
    ///   Expected number of back buffers of this swap chain. This value will always be clamped
    ///   between 2 and 3.
    /// @param bufferFormat
    ///   Back buffer pixel format.
    /// @param enableTearing
    ///   Specified whether to enable variable refresh rate for this swap chain.
    InkApi SwapChain(HWND          window,
                     std::uint32_t numBuffers    = 2,
                     DXGI_FORMAT   bufferFormat  = DXGI_FORMAT_R8G8B8A8_UNORM,
                     bool          enableTearing = false) noexcept;

    /// @brief
    ///   Copy constructor for swap chain is disabled.
    SwapChain(const SwapChain &) = delete;

    /// @brief
    ///   Copy assignment for swap chain is disabled.
    auto operator=(const SwapChain &) = delete;

    /// @brief
    ///   Move constructor of swap chain.
    ///
    /// @param other
    ///   The swap chain to be moved. The moved swap chain will be invalidated.
    InkApi SwapChain(SwapChain &&other) noexcept;

    /// @brief
    ///   Move assignment of swap chain.
    ///
    /// @param other
    ///   The swap chain to be moved. The moved swap chain will be invalidated.
    ///
    /// @return
    ///   Reference to this swap chain.
    InkApi auto operator=(SwapChain &&other) noexcept -> SwapChain &;

    /// @brief
    ///   Destroy this swap chain.
    InkApi ~SwapChain() noexcept;

    /// @brief
    ///   Present current back buffer.
    /// @note
    ///   This method returns immediately and will not block current thread.
    InkApi auto present() noexcept -> void;

    /// @brief
    ///   Resize swap chain back buffers.
    /// @note
    ///   This method will perform a synchonization operation between CPU and GPU to wait for all
    ///   back buffers idle. Errors will be handled with assertions.
    ///
    /// @param width
    ///   New width of back buffers. Pass (0, 0) to use window client size as new back buffer size.
    /// @param height
    ///   New height of back buffers. Pass (0, 0) to use window client size as new back buffer size.
    InkApi auto resize(std::uint32_t width, std::uint32_t height) noexcept -> void;

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
    [[nodiscard]]
    auto isTearingEnabled() const noexcept -> bool {
        return m_isTearingEnabled;
    }

    /// @brief
    ///   Wait for current back buffer available and get current back buffer.
    /// @note
    ///   This method will block current thread until current back buffer is available.
    ///
    /// @return
    ///   Reference to current back buffer.
    [[nodiscard]]
    auto backBuffer() const noexcept -> ColorBuffer & {
        m_renderDevice->sync(m_presentFenceValues[m_bufferIndex]);
        return m_backBuffers[m_bufferIndex];
    }

    /// @brief
    ///   Get back buffer pixel format.
    ///
    /// @return
    ///   Back buffer pixel format.
    [[nodiscard]]
    auto pixelFormat() const noexcept -> DXGI_FORMAT {
        return m_pixelFormat;
    }

    /// @brief
    ///   Get total number of back buffers in this swap chain.
    ///
    /// @return
    ///   Number of back buffers in this swap chain.
    [[nodiscard]]
    auto bufferCount() const noexcept -> std::uint32_t {
        return m_bufferCount;
    }

    /// @brief
    ///   Set clear color for all back buffers.
    ///
    /// @param color
    ///   New clear color to be set for back buffers.
    auto setClearColor(const Color &color) noexcept -> void {
        for (auto &buffer : m_backBuffers)
            buffer.clearColor(color);
    }

private:
    /// @brief
    ///   The render device that is used to create this swap chain.
    RenderDevice *m_renderDevice;

    /// @brief
    ///   The D3D12 swap chain object.
    Microsoft::WRL::ComPtr<IDXGISwapChain1> m_swapChain;

    /// @brief
    ///   Specifies whether tearing is enabled for this swap chain.
    bool m_isTearingEnabled;

    /// @brief
    ///   Number of back buffers in this swap chain. Must be 2 or 3.
    std::uint32_t m_bufferCount;

    /// @brief
    ///   Index of current back buffer.
    std::uint32_t m_bufferIndex;

    /// @brief
    ///   Back buffer pixel format.
    DXGI_FORMAT m_pixelFormat;

    /// @brief
    ///   Back buffers of this swap chain.
    mutable ColorBuffer m_backBuffers[3];

    /// @brief
    ///   Fence value that indicates when the corresponding back buffer could be reused.
    std::uint64_t m_presentFenceValues[3];
};

} // namespace ink

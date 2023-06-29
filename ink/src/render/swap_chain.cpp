#include "ink/render/swap_chain.h"
#include "ink/core/assert.h"

using namespace ink;
using Microsoft::WRL::ComPtr;

ink::SwapChain::SwapChain() noexcept
    : m_renderDevice(nullptr),
      m_swapChain(),
      m_isTearingEnabled(),
      m_bufferCount(),
      m_bufferIndex(),
      m_pixelFormat(DXGI_FORMAT_UNKNOWN),
      m_backBuffers(),
      m_presentFenceValues() {}

ink::SwapChain::SwapChain(HWND          window,
                          std::uint32_t numBuffers,
                          DXGI_FORMAT   bufferFormat,
                          bool          enableTearing) noexcept
    : m_renderDevice(&RenderDevice::singleton()),
      m_swapChain(),
      m_isTearingEnabled(),
      m_bufferCount(numBuffers > 2 ? 3 : 2),
      m_bufferIndex(0),
      m_pixelFormat(bufferFormat),
      m_backBuffers(),
      m_presentFenceValues() {
    IDXGIFactory6 *const dxgiFactory = m_renderDevice->factory();

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

    UINT flags = 0;
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

    [[maybe_unused]] HRESULT hr;
    hr = dxgiFactory->CreateSwapChainForHwnd(m_renderDevice->commandQueue(), window, &desc, nullptr,
                                             nullptr, m_swapChain.GetAddressOf());
    inkAssert(SUCCEEDED(hr), u"Failed to create swap chain: 0x{:X}.",
              static_cast<std::uint32_t>(hr));

    // Disable Alt-Enter
    dxgiFactory->MakeWindowAssociation(window, DXGI_MWA_NO_ALT_ENTER);

    // Get back buffers.
    for (std::uint32_t i = 0; i < m_bufferCount; ++i) {
        ComPtr<ID3D12Resource> backBuffer;
        hr = m_swapChain->GetBuffer(i, IID_PPV_ARGS(backBuffer.GetAddressOf()));
        inkAssert(SUCCEEDED(hr), u"Failed to get the {}'th back buffer from swap chain: 0x{:X}.", i,
                  static_cast<std::uint32_t>(hr));

        m_backBuffers[i].resetSwapChainBuffer(std::move(backBuffer));
    }
}

ink::SwapChain::SwapChain(SwapChain &&other) noexcept = default;

auto ink::SwapChain::operator=(SwapChain &&other) noexcept -> SwapChain & = default;

ink::SwapChain::~SwapChain() noexcept {
    if (m_swapChain != nullptr)
        m_renderDevice->sync();
}

auto ink::SwapChain::present() noexcept -> void {
    m_swapChain->Present(0, m_isTearingEnabled ? DXGI_PRESENT_ALLOW_TEARING : 0);

    // Acquire a fence value.
    const std::uint64_t fenceValue      = m_renderDevice->signalFence();
    m_presentFenceValues[m_bufferIndex] = fenceValue;

    // Increase buffer index.
    m_bufferIndex = (m_bufferIndex + 1) % m_bufferCount;
}

auto ink::SwapChain::resize(std::uint32_t width, std::uint32_t height) noexcept -> void {
    m_renderDevice->sync();

    // Release back buffers.
    for (std::uint32_t i = 0; i < m_bufferCount; ++i)
        m_backBuffers[i].releaseSwapChainResource();

    // Resize back buffers.
    [[maybe_unused]] HRESULT hr;

    UINT flags = m_isTearingEnabled ? DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING : 0;
    hr         = m_swapChain->ResizeBuffers(0, width, height, DXGI_FORMAT_UNKNOWN, flags);
    inkAssert(SUCCEEDED(hr), u"Failed to resize swap chain back buffers to size ({}, {}): 0x{:X}.",
              width, height, static_cast<std::uint32_t>(hr));

    // Get back buffers.
    for (std::uint32_t i = 0; i < m_bufferCount; ++i) {
        ComPtr<ID3D12Resource> backBuffer;
        hr = m_swapChain->GetBuffer(i, IID_PPV_ARGS(backBuffer.GetAddressOf()));
        inkAssert(SUCCEEDED(hr), u"Failed to get the {}'th back buffer from swap chain: 0x{:X}.", i,
                  static_cast<std::uint32_t>(hr));

        m_backBuffers[i].resetSwapChainBuffer(std::move(backBuffer));
    }

    m_bufferIndex = 0;
}

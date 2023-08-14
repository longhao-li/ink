#include "ink/render/descriptor.hpp"
#include "ink/render/device.hpp"

using namespace ink;

ink::ConstantBufferView::ConstantBufferView() noexcept : m_renderDevice(nullptr), m_handle() {}

ink::ConstantBufferView::ConstantBufferView(ConstantBufferView &&other) noexcept
    : m_renderDevice(other.m_renderDevice), m_handle(other.m_handle) {
    other.m_renderDevice = nullptr;
    other.m_handle       = {};
}

ink::ConstantBufferView::~ConstantBufferView() noexcept {
    if (!m_handle.isNull())
        m_renderDevice->releaseConstantBufferViewDescriptor(m_handle.value());
}

auto ink::ConstantBufferView::operator=(ConstantBufferView &&other) noexcept
    -> ConstantBufferView & {
    if (this == &other)
        return *this;

    if (!m_handle.isNull())
        m_renderDevice->releaseConstantBufferViewDescriptor(m_handle.value());

    m_renderDevice = other.m_renderDevice;
    m_handle       = other.m_handle;

    other.m_renderDevice = nullptr;
    other.m_handle       = {};

    return *this;
}

auto ink::ConstantBufferView::update(std::uint64_t gpuAddress, std::uint32_t size) noexcept
    -> void {
    assert(!m_handle.isNull());

    const D3D12_CONSTANT_BUFFER_VIEW_DESC desc{
        /* BufferLocation = */ gpuAddress,
        /* SizeInBytes    = */ size,
    };

    m_renderDevice->m_device->CreateConstantBufferView(&desc, m_handle);
}

auto ink::ConstantBufferView::update(ID3D12Resource                        *resource,
                                     const D3D12_SHADER_RESOURCE_VIEW_DESC *desc) noexcept -> void {
    assert(!m_handle.isNull());
    m_renderDevice->m_device->CreateShaderResourceView(resource, desc, m_handle);
}

auto ink::ConstantBufferView::update(ID3D12Resource                         *resource,
                                     ID3D12Resource                         *counter,
                                     const D3D12_UNORDERED_ACCESS_VIEW_DESC *desc) noexcept
    -> void {
    assert(!m_handle.isNull());
    m_renderDevice->m_device->CreateUnorderedAccessView(resource, counter, desc, m_handle);
}

ink::RenderTargetView::RenderTargetView() noexcept : m_renderDevice(nullptr), m_handle() {}

ink::RenderTargetView::RenderTargetView(RenderTargetView &&other) noexcept
    : m_renderDevice(other.m_renderDevice), m_handle(other.m_handle) {
    other.m_renderDevice = nullptr;
    other.m_handle       = {};
}

ink::RenderTargetView::~RenderTargetView() noexcept {
    if (!m_handle.isNull())
        m_renderDevice->releaseRenderTargetViewDescriptor(m_handle.value());
}

auto ink::RenderTargetView::operator=(RenderTargetView &&other) noexcept -> RenderTargetView & {
    if (this == &other)
        return *this;

    if (!m_handle.isNull())
        m_renderDevice->releaseRenderTargetViewDescriptor(m_handle.value());

    m_renderDevice = other.m_renderDevice;
    m_handle       = other.m_handle;

    other.m_renderDevice = nullptr;
    other.m_handle       = {};

    return *this;
}

auto ink::RenderTargetView::update(ID3D12Resource                      *resource,
                                   const D3D12_RENDER_TARGET_VIEW_DESC *desc) noexcept -> void {
    assert(!m_handle.isNull());
    m_renderDevice->m_device->CreateRenderTargetView(resource, desc, m_handle);
}

ink::DepthStencilView::DepthStencilView() noexcept : m_renderDevice(nullptr), m_handle() {}

ink::DepthStencilView::DepthStencilView(DepthStencilView &&other) noexcept
    : m_renderDevice(other.m_renderDevice), m_handle(other.m_handle) {
    other.m_renderDevice = nullptr;
    other.m_handle       = {};
}

ink::DepthStencilView::~DepthStencilView() noexcept {
    if (!m_handle.isNull())
        m_renderDevice->releaseDepthStencilViewDescriptor(m_handle.value());
}

auto ink::DepthStencilView::operator=(DepthStencilView &&other) noexcept -> DepthStencilView & {
    if (this == &other)
        return *this;

    if (!m_handle.isNull())
        m_renderDevice->releaseDepthStencilViewDescriptor(m_handle.value());

    m_renderDevice = other.m_renderDevice;
    m_handle       = other.m_handle;

    other.m_renderDevice = nullptr;
    other.m_handle       = {};

    return *this;
}

auto ink::DepthStencilView::update(ID3D12Resource                      *resource,
                                   const D3D12_DEPTH_STENCIL_VIEW_DESC *desc) noexcept -> void {
    assert(!m_handle.isNull());
    m_renderDevice->m_device->CreateDepthStencilView(resource, desc, m_handle);
}

ink::Sampler::Sampler() noexcept : m_renderDevice(), m_handle(), m_desc() {}

ink::Sampler::Sampler(Sampler &&other) noexcept
    : m_renderDevice(other.m_renderDevice), m_handle(other.m_handle), m_desc(other.m_desc) {
    other.m_renderDevice = nullptr;
    other.m_handle       = {};
    other.m_desc         = {};
}

ink::Sampler::~Sampler() noexcept {
    if (!m_handle.isNull())
        m_renderDevice->releaseSamplerDescriptor(m_handle.value());
}

auto ink::Sampler::operator=(Sampler &&other) noexcept -> Sampler & {
    if (this == &other)
        return *this;

    if (!m_handle.isNull())
        m_renderDevice->releaseSamplerDescriptor(m_handle.value());

    m_renderDevice = other.m_renderDevice;
    m_handle       = other.m_handle;
    m_desc         = other.m_desc;

    other.m_renderDevice = nullptr;
    other.m_handle       = {};
    other.m_desc         = {};

    return *this;
}

auto ink::Sampler::update(const D3D12_SAMPLER_DESC &desc) noexcept -> void {
    assert(!m_handle.isNull());
    m_renderDevice->m_device->CreateSampler(&desc, m_handle);
    m_desc = desc;
}

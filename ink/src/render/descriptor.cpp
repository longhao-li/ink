#include "ink/render/descriptor.h"
#include "ink/render/device.h"

using namespace ink;

ink::ConstantBufferView::ConstantBufferView(const ConstantBufferView &other) noexcept : m_handle() {
    if (other.isNull())
        return;

    auto &dev = RenderDevice::singleton();
    m_handle  = dev.newConstantBufferView();

    dev.device()->CopyDescriptorsSimple(1, m_handle, other.m_handle,
                                        D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
}

auto ink::ConstantBufferView::operator=(const ConstantBufferView &other) noexcept
    -> ConstantBufferView & {
    if (this == &other)
        return *this;

    if (other.isNull()) {
        if (!m_handle.isNull()) {
            auto &dev = RenderDevice::singleton();
            dev.freeConstantBufferView(m_handle);
            m_handle = CpuDescriptorHandle();
        }

        return *this;
    }

    auto &dev = RenderDevice::singleton();
    if (m_handle.isNull())
        m_handle = dev.newConstantBufferView();

    dev.device()->CopyDescriptorsSimple(1, m_handle, other.m_handle,
                                        D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

    return *this;
}

auto ink::ConstantBufferView::operator=(ConstantBufferView &&other) noexcept
    -> ConstantBufferView & {
    if (!m_handle.isNull()) {
        auto &dev = RenderDevice::singleton();
        dev.freeConstantBufferView(m_handle);
    }

    m_handle       = other.m_handle;
    other.m_handle = CpuDescriptorHandle();

    return *this;
}

ink::ConstantBufferView::~ConstantBufferView() noexcept {
    if (!m_handle.isNull()) {
        auto &dev = RenderDevice::singleton();
        dev.freeConstantBufferView(m_handle);
    }
}

auto ink::ConstantBufferView::initConstantBuffer(std::uint64_t gpuAddress,
                                                 std::uint32_t size) noexcept -> void {
    auto &dev = RenderDevice::singleton();
    if (m_handle.isNull())
        m_handle = dev.newConstantBufferView();

    const D3D12_CONSTANT_BUFFER_VIEW_DESC desc{
        /* BufferLocation = */ gpuAddress,
        /* SizeInBytes    = */ size,
    };

    dev.device()->CreateConstantBufferView(&desc, m_handle);
}

auto ink::ConstantBufferView::initConstantBuffer(
    const D3D12_CONSTANT_BUFFER_VIEW_DESC &desc) noexcept -> void {
    auto &dev = RenderDevice::singleton();
    if (m_handle.isNull())
        m_handle = dev.newConstantBufferView();

    dev.device()->CreateConstantBufferView(&desc, m_handle);
}

auto ink::ConstantBufferView::initShaderResource(
    ID3D12Resource *resource, const D3D12_SHADER_RESOURCE_VIEW_DESC *desc) noexcept -> void {
    auto &dev = RenderDevice::singleton();
    if (m_handle.isNull())
        m_handle = dev.newConstantBufferView();

    dev.device()->CreateShaderResourceView(resource, desc, m_handle);
}

auto ink::ConstantBufferView::initUnorderedAccess(
    ID3D12Resource                         *resource,
    ID3D12Resource                         *counter,
    const D3D12_UNORDERED_ACCESS_VIEW_DESC *desc) noexcept -> void {
    auto &dev = RenderDevice::singleton();
    if (m_handle.isNull())
        m_handle = dev.newConstantBufferView();

    dev.device()->CreateUnorderedAccessView(resource, counter, desc, m_handle);
}

ink::SamplerView::SamplerView(const SamplerView &other) noexcept : m_handle() {
    if (other.isNull())
        return;

    auto &dev = RenderDevice::singleton();
    m_handle  = dev.newSamplerView();

    dev.device()->CopyDescriptorsSimple(1, m_handle, other.m_handle,
                                        D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER);
}

auto ink::SamplerView::operator=(const SamplerView &other) noexcept -> SamplerView & {
    if (this == &other)
        return *this;

    if (other.isNull()) {
        if (!m_handle.isNull()) {
            auto &dev = RenderDevice::singleton();
            dev.freeSamplerView(m_handle);
            m_handle = CpuDescriptorHandle();
        }

        return *this;
    }

    auto &dev = RenderDevice::singleton();
    m_handle  = dev.newSamplerView();

    dev.device()->CopyDescriptorsSimple(1, m_handle, other.m_handle,
                                        D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER);

    return *this;
}

auto ink::SamplerView::operator=(SamplerView &&other) noexcept -> SamplerView & {
    if (!m_handle.isNull()) {
        auto &dev = RenderDevice::singleton();
        dev.freeSamplerView(m_handle);
    }

    m_handle       = other.m_handle;
    other.m_handle = CpuDescriptorHandle();

    return *this;
}

ink::SamplerView::~SamplerView() noexcept {
    if (!m_handle.isNull()) {
        auto &dev = RenderDevice::singleton();
        dev.freeSamplerView(m_handle);
    }
}

auto ink::SamplerView::initSampler(const D3D12_SAMPLER_DESC &desc) noexcept -> void {
    auto &dev = RenderDevice::singleton();
    if (m_handle.isNull())
        m_handle = dev.newSamplerView();

    dev.device()->CreateSampler(&desc, m_handle);
}

ink::RenderTargetView::RenderTargetView(const RenderTargetView &other) noexcept : m_handle() {
    if (other.isNull())
        return;

    auto &dev = RenderDevice::singleton();
    m_handle  = dev.newRenderTargetView();

    dev.device()->CopyDescriptorsSimple(1, m_handle, other.m_handle,
                                        D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
}

auto ink::RenderTargetView::operator=(const RenderTargetView &other) noexcept
    -> RenderTargetView & {
    if (this == &other)
        return *this;

    if (other.isNull()) {
        if (!m_handle.isNull()) {
            auto &dev = RenderDevice::singleton();
            dev.freeRenderTargetView(m_handle);
            m_handle = CpuDescriptorHandle();
        }

        return *this;
    }

    auto &dev = RenderDevice::singleton();
    m_handle  = dev.newRenderTargetView();

    dev.device()->CopyDescriptorsSimple(1, m_handle, other.m_handle,
                                        D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

    return *this;
}

auto ink::RenderTargetView::operator=(RenderTargetView &&other) noexcept -> RenderTargetView & {
    if (!m_handle.isNull()) {
        auto &dev = RenderDevice::singleton();
        dev.freeRenderTargetView(m_handle);
    }

    m_handle       = other.m_handle;
    other.m_handle = CpuDescriptorHandle();

    return *this;
}

ink::RenderTargetView::~RenderTargetView() noexcept {
    if (!m_handle.isNull()) {
        auto &dev = RenderDevice::singleton();
        dev.freeRenderTargetView(m_handle);
    }
}

auto ink::RenderTargetView::initRenderTarget(ID3D12Resource                      *resource,
                                             const D3D12_RENDER_TARGET_VIEW_DESC *desc) noexcept
    -> void {
    auto &dev = RenderDevice::singleton();
    if (m_handle.isNull())
        m_handle = dev.newRenderTargetView();

    dev.device()->CreateRenderTargetView(resource, desc, m_handle);
}

ink::DepthStencilView::DepthStencilView(const DepthStencilView &other) noexcept : m_handle() {
    if (other.isNull())
        return;

    auto &dev = RenderDevice::singleton();
    m_handle  = dev.newRenderTargetView();

    dev.device()->CopyDescriptorsSimple(1, m_handle, other.m_handle,
                                        D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
}

auto ink::DepthStencilView::operator=(const DepthStencilView &other) noexcept
    -> DepthStencilView & {
    if (this == &other)
        return *this;

    if (other.isNull()) {
        if (!m_handle.isNull()) {
            auto &dev = RenderDevice::singleton();
            dev.freeDepthStencilView(m_handle);
            m_handle = CpuDescriptorHandle();
        }

        return *this;
    }

    auto &dev = RenderDevice::singleton();
    m_handle  = dev.newDepthStencilView();

    dev.device()->CopyDescriptorsSimple(1, m_handle, other.m_handle,
                                        D3D12_DESCRIPTOR_HEAP_TYPE_DSV);

    return *this;
}

auto ink::DepthStencilView::operator=(DepthStencilView &&other) noexcept -> DepthStencilView & {
    if (!m_handle.isNull()) {
        auto &dev = RenderDevice::singleton();
        dev.freeDepthStencilView(m_handle);
    }

    m_handle       = other.m_handle;
    other.m_handle = CpuDescriptorHandle();

    return *this;
}

ink::DepthStencilView::~DepthStencilView() noexcept {
    if (!m_handle.isNull()) {
        auto &dev = RenderDevice::singleton();
        dev.freeDepthStencilView(m_handle);
    }
}

auto ink::DepthStencilView::initDepthStencil(ID3D12Resource                      *resource,
                                             const D3D12_DEPTH_STENCIL_VIEW_DESC *desc) noexcept
    -> void {
    auto &dev = RenderDevice::singleton();
    if (m_handle.isNull())
        m_handle = dev.newDepthStencilView();

    dev.device()->CreateDepthStencilView(resource, desc, m_handle);
}

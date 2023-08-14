#include "ink/render/command_buffer.hpp"
#include "ink/core/exception.hpp"
#include "ink/render/device.hpp"

using namespace ink;

ink::DynamicBufferPage::DynamicBufferPage(ID3D12Device *device, std::size_t size)
    : GpuResource(), m_size(size), m_data(), m_gpuAddress() {
    // Create ID3D12Resource.
    const D3D12_HEAP_PROPERTIES heapProps{
        /* Type                 = */ D3D12_HEAP_TYPE_UPLOAD,
        /* CPUPageProperty      = */ D3D12_CPU_PAGE_PROPERTY_UNKNOWN,
        /* MemoryPoolPreference = */ D3D12_MEMORY_POOL_UNKNOWN,
        /* CreationNodeMask     = */ 0,
        /* VisibleNodeMask      = */ 0,
    };

    const D3D12_RESOURCE_DESC desc{
        /* Dimension        = */ D3D12_RESOURCE_DIMENSION_BUFFER,
        /* Alignment        = */ 0,
        /* Width            = */ static_cast<UINT64>(size),
        /* Height           = */ 1,
        /* DepthOrArraySize = */ 1,
        /* MipLevels        = */ 1,
        /* Format           = */ DXGI_FORMAT_UNKNOWN,
        /* SampleDesc       = */
        {
            /* Count   = */ 1,
            /* Quality = */ 0,
        },
        /* Layout = */ D3D12_TEXTURE_LAYOUT_ROW_MAJOR,
        /* Flags  = */ D3D12_RESOURCE_FLAG_NONE,
    };

    HRESULT hr = device->CreateCommittedResource(&heapProps, D3D12_HEAP_FLAG_NONE, &desc,
                                                 D3D12_RESOURCE_STATE_GENERIC_READ, nullptr,
                                                 IID_PPV_ARGS(m_resource.GetAddressOf()));
    if (FAILED(hr))
        throw RenderAPIException(hr, "Failed to create ID3D12Resource for dynamic buffer page.");

    m_usageState = D3D12_RESOURCE_STATE_GENERIC_READ;
    m_resource->Map(0, nullptr, &m_data);
    m_gpuAddress = m_resource->GetGPUVirtualAddress();
}

ink::DynamicBufferPage::DynamicBufferPage(DynamicBufferPage &&other) noexcept
    : GpuResource(std::move(other)),
      m_size(other.m_size),
      m_data(other.m_data),
      m_gpuAddress(other.m_gpuAddress) {
    other.m_data = nullptr;
}

ink::DynamicBufferPage::~DynamicBufferPage() noexcept {
    if (m_data != nullptr)
        m_resource->Unmap(0, nullptr);
}

auto ink::DynamicBufferPage::operator=(DynamicBufferPage &&other) noexcept -> DynamicBufferPage & {
    if (this == &other)
        return *this;

    if (m_data != nullptr)
        m_resource->Unmap(0, nullptr);

    GpuResource::operator=(std::move(other));
    m_size       = other.m_size;
    m_data       = other.m_data;
    m_gpuAddress = other.m_gpuAddress;

    other.m_data = nullptr;

    return *this;
}

ink::DynamicBufferAllocator::DynamicBufferAllocator(RenderDevice &renderDevice) noexcept
    : m_renderDevice(&renderDevice), m_offset(), m_page(nullptr), m_retiredPages() {}

ink::DynamicBufferAllocator::DynamicBufferAllocator() noexcept
    : m_renderDevice(nullptr), m_offset(), m_page(nullptr), m_retiredPages() {}

ink::DynamicBufferAllocator::DynamicBufferAllocator(DynamicBufferAllocator &&other) noexcept
    : m_renderDevice(other.m_renderDevice),
      m_offset(other.m_offset),
      m_page(other.m_page),
      m_retiredPages(std::move(other.m_retiredPages)) {
    other.m_offset = 0;
    other.m_page   = nullptr;
}

ink::DynamicBufferAllocator::~DynamicBufferAllocator() noexcept {
    if (m_page != nullptr)
        m_retiredPages.push_back(m_page);

    if (!m_retiredPages.empty())
        m_renderDevice->releaseDynamicBufferPages(m_renderDevice->signalFence(),
                                                  m_retiredPages.size(), m_retiredPages.data());
}

auto ink::DynamicBufferAllocator::operator=(DynamicBufferAllocator &&other) noexcept
    -> DynamicBufferAllocator & {
    if (this == &other)
        return *this;

    if (m_page != nullptr)
        m_retiredPages.push_back(m_page);

    if (!m_retiredPages.empty())
        m_renderDevice->releaseDynamicBufferPages(m_renderDevice->signalFence(),
                                                  m_retiredPages.size(), m_retiredPages.data());

    m_renderDevice = other.m_renderDevice;
    m_offset       = other.m_offset;
    m_page         = other.m_page;
    m_retiredPages = std::move(other.m_retiredPages);

    other.m_offset = 0;
    other.m_page   = nullptr;

    return *this;
}

namespace {

/// @brief
///   Default dynamic buffer page size is 16Mib.
constexpr const std::size_t DYNAMIC_BUFFER_PAGE_SIZE = 0x1000000;

} // namespace

auto ink::DynamicBufferAllocator::allocate(std::size_t size, std::size_t alignment)
    -> DynamicBufferAllocation {
    // Align up allocate size.
    assert((alignment & (alignment - 1)) == 0 && "Alignment must be a power of 2.");
    size = (size + alignment - 1) & ~(alignment - 1);

    // Allocate a single page if size is greater than default page size.
    if (size >= DYNAMIC_BUFFER_PAGE_SIZE) {
        DynamicBufferPage *page = m_renderDevice->acquireDynamicBufferPage(size);
        m_retiredPages.push_back(page);

        return {
            /* resource   = */ page,
            /* size       = */ size,
            /* offset     = */ 0,
            /* data       = */ page->map<void>(),
            /* gpuAddress = */ page->gpuAddress(),
        };
    }

    // Align up offset.
    std::size_t extraSize = 0;
    if (m_page != nullptr) {
        extraSize = ((m_offset + alignment - 1) & (alignment - 1)) - m_offset;
        if (m_offset + size + extraSize > DYNAMIC_BUFFER_PAGE_SIZE) {
            m_retiredPages.push_back(m_page);
            m_page = nullptr;
        }
    }

    // Allocate a new page if current page is null.
    if (m_page == nullptr) {
        m_page    = m_renderDevice->acquireDynamicBufferPage(DYNAMIC_BUFFER_PAGE_SIZE);
        m_offset  = 0;
        extraSize = 0;
    }

    DynamicBufferAllocation allocation{
        /* resource   = */ m_page,
        /* size       = */ size,
        /* offset     = */ m_offset + extraSize,
        /* data       = */ m_page->map<std::uint8_t>() + m_offset + extraSize,
        /* gpuAddress = */ m_page->gpuAddress() + m_offset + extraSize,
    };

    m_offset += size + extraSize;
    return allocation;
}

auto ink::DynamicBufferAllocator::reset(std::uint64_t fenceValue) noexcept -> void {
    if (!m_retiredPages.empty()) {
        m_renderDevice->releaseDynamicBufferPages(fenceValue, m_retiredPages.size(),
                                                  m_retiredPages.data());
        m_retiredPages.clear();
    }
}

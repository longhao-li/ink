#include "ink/render/command_buffer.h"
#include "ink/core/assert.h"
#include "ink/render/device.h"
#include "ink/render/pipeline.h"

#include <stack>

using namespace ink;

ink::DynamicDescriptorHeap::DynamicDescriptorHeap() noexcept
    : m_renderDevice(),
      m_device(),
      m_heapType(),
      m_descriptorSize(),
      m_graphicsRootSignature(),
      m_computeRootSignature(),
      m_currentHeap(),
      m_currentHandle(),
      m_freeDescriptorCount(),
      m_retiredHeaps(),
      m_graphicsDescriptors(),
      m_computeDescriptors(),
      m_graphicsTableRange(),
      m_computeTableRange() {}

ink::DynamicDescriptorHeap::DynamicDescriptorHeap(
    D3D12_DESCRIPTOR_HEAP_TYPE descriptorType) noexcept
    : m_renderDevice(&RenderDevice::singleton()),
      m_device(m_renderDevice->device()),
      m_heapType(descriptorType),
      m_descriptorSize(m_device->GetDescriptorHandleIncrementSize(descriptorType)),
      m_graphicsRootSignature(),
      m_computeRootSignature(),
      m_currentHeap(),
      m_currentHandle(),
      m_freeDescriptorCount(),
      m_retiredHeaps(),
      m_graphicsDescriptors(),
      m_computeDescriptors(),
      m_graphicsTableRange(),
      m_computeTableRange() {}

ink::DynamicDescriptorHeap::DynamicDescriptorHeap(DynamicDescriptorHeap &&other) noexcept
    : m_renderDevice(other.m_renderDevice),
      m_device(other.m_device),
      m_heapType(other.m_heapType),
      m_descriptorSize(other.m_descriptorSize),
      m_graphicsRootSignature(other.m_graphicsRootSignature),
      m_computeRootSignature(other.m_computeRootSignature),
      m_currentHeap(other.m_currentHeap),
      m_currentHandle(other.m_currentHandle),
      m_freeDescriptorCount(other.m_freeDescriptorCount),
      m_retiredHeaps(std::move(other.m_retiredHeaps)),
      m_graphicsDescriptors(std::move(other.m_graphicsDescriptors)),
      m_computeDescriptors(std::move(other.m_computeDescriptors)) {
    std::memcpy(m_graphicsTableRange, other.m_graphicsTableRange, sizeof(m_graphicsTableRange));
    std::memcpy(m_computeTableRange, other.m_computeTableRange, sizeof(m_computeTableRange));

    other.m_graphicsRootSignature = nullptr;
    other.m_computeRootSignature  = nullptr;
    other.m_currentHeap           = nullptr;
    other.m_currentHandle         = DescriptorHandle();
    other.m_freeDescriptorCount   = 0;
    std::memset(other.m_graphicsTableRange, 0, sizeof(other.m_graphicsTableRange));
    std::memset(other.m_computeTableRange, 0, sizeof(other.m_computeTableRange));
}

ink::DynamicDescriptorHeap::~DynamicDescriptorHeap() noexcept {
    if (m_currentHeap != nullptr)
        m_retiredHeaps.push_back(m_currentHeap);

    if (!m_retiredHeaps.empty()) {
        std::uint64_t fenceValue = m_renderDevice->signalFence();
        if (m_heapType == D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV)
            m_renderDevice->freeDynamicViewHeaps(fenceValue, m_retiredHeaps.size(),
                                                 m_retiredHeaps.data());
        else
            m_renderDevice->freeDynamicSamplerHeaps(fenceValue, m_retiredHeaps.size(),
                                                    m_retiredHeaps.data());
    }
}

auto ink::DynamicDescriptorHeap::reset(std::uint64_t fenceValue) noexcept -> void {
    if (!m_retiredHeaps.empty()) {

        if (m_heapType == D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV)
            m_renderDevice->freeDynamicViewHeaps(fenceValue, m_retiredHeaps.size(),
                                                 m_retiredHeaps.data());
        else
            m_renderDevice->freeDynamicSamplerHeaps(fenceValue, m_retiredHeaps.size(),
                                                    m_retiredHeaps.data());

        m_retiredHeaps.clear();
    }

    // Reset root signatures.
    m_graphicsRootSignature = nullptr;
    m_computeRootSignature  = nullptr;

    // Clear descriptions.
    m_graphicsDescriptors.clear();
    m_computeDescriptors.clear();
    std::memset(m_graphicsTableRange, 0, sizeof(m_graphicsTableRange));
    std::memset(m_computeTableRange, 0, sizeof(m_computeTableRange));
}

auto ink::DynamicDescriptorHeap::parseGraphicsRootSignature(const RootSignature &rootSig) noexcept
    -> void {
    m_graphicsRootSignature = &rootSig;

    const auto descCount =
        (m_heapType == D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER ? rootSig.tableSamplerCount()
                                                          : rootSig.tableViewCount());
    m_graphicsDescriptors.resize(descCount);
    std::memset(m_graphicsDescriptors.data(), 0, descCount * sizeof(DescriptorCache));

    std::uint32_t offset = 0;
    if (m_heapType == D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER) {
        for (std::uint32_t i = 0; i < std::size(m_graphicsTableRange); ++i) {
            if (!rootSig.isSamplerTable(i))
                continue;

            std::uint32_t tableSize       = rootSig.tableSize(i);
            m_graphicsTableRange[i].start = static_cast<std::uint16_t>(offset);
            m_graphicsTableRange[i].count = static_cast<std::uint16_t>(tableSize);
            offset += tableSize;
        }
    } else {
        for (std::uint32_t i = 0; i < std::size(m_graphicsTableRange); ++i) {
            if (!rootSig.isViewTable(i))
                continue;

            std::uint32_t tableSize       = rootSig.tableSize(i);
            m_graphicsTableRange[i].start = static_cast<std::uint16_t>(offset);
            m_graphicsTableRange[i].count = static_cast<std::uint16_t>(tableSize);
            offset += tableSize;
        }
    }
}

auto ink::DynamicDescriptorHeap::parseComputeRootSignature(const RootSignature &rootSig) noexcept
    -> void {
    m_computeRootSignature = &rootSig;

    const auto descCount =
        (m_heapType == D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER ? rootSig.tableSamplerCount()
                                                          : rootSig.tableViewCount());

    m_computeDescriptors.resize(descCount);
    std::memset(m_computeDescriptors.data(), 0, descCount * sizeof(DescriptorCache));

    std::uint32_t offset = 0;
    if (m_heapType == D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER) {
        for (std::uint32_t i = 0; i < std::size(m_computeTableRange); ++i) {
            if (!rootSig.isSamplerTable(i))
                continue;

            std::uint32_t tableSize      = rootSig.tableSize(i);
            m_computeTableRange[i].start = static_cast<std::uint16_t>(offset);
            m_computeTableRange[i].count = static_cast<std::uint16_t>(tableSize);
            offset += tableSize;
        }
    } else {
        for (std::uint32_t i = 0; i < std::size(m_computeTableRange); ++i) {
            if (!rootSig.isViewTable(i))
                continue;

            std::uint32_t tableSize      = rootSig.tableSize(i);
            m_computeTableRange[i].start = static_cast<std::uint16_t>(offset);
            m_computeTableRange[i].count = static_cast<std::uint16_t>(tableSize);
            offset += tableSize;
        }
    }
}

auto ink::DynamicDescriptorHeap::bindGraphicsDescriptor(std::uint32_t       paramIndex,
                                                        std::uint32_t       offset,
                                                        CpuDescriptorHandle descriptor) noexcept
    -> void {
    inkAssert(paramIndex < std::size(m_graphicsTableRange), u"Root parameter index out of range.");

    auto &table = m_graphicsTableRange[paramIndex];
    inkAssert(offset < table.count, u"Descriptor offset out of range.");

    auto &param     = m_graphicsDescriptors[static_cast<std::size_t>(table.start) + offset];
    param.cacheType = CacheType::CpuDescriptor;
    param.handle    = descriptor;
}

auto ink::DynamicDescriptorHeap::bindGraphicsDescriptor(
    std::uint32_t                          paramIndex,
    std::uint32_t                          offset,
    const D3D12_CONSTANT_BUFFER_VIEW_DESC &desc) noexcept -> void {
    inkAssert(paramIndex < std::size(m_graphicsTableRange), u"Root parameter index out of range.");

    auto &table = m_graphicsTableRange[paramIndex];
    inkAssert(offset < table.count, u"Descriptor offset out of range.");

    auto &param          = m_graphicsDescriptors[static_cast<std::size_t>(table.start) + offset];
    param.cacheType      = CacheType::ConstantBufferView;
    param.constantBuffer = desc;
}

auto ink::DynamicDescriptorHeap::bindComputeDescriptor(std::uint32_t       paramIndex,
                                                       std::uint32_t       offset,
                                                       CpuDescriptorHandle descriptor) noexcept
    -> void {
    inkAssert(paramIndex < std::size(m_computeTableRange), u"Root parameter index out of range.");

    auto &table = m_computeTableRange[paramIndex];
    inkAssert(offset < table.count, u"Descriptor offset out of range.");

    auto &param     = m_computeDescriptors[static_cast<std::size_t>(table.start) + offset];
    param.cacheType = CacheType::CpuDescriptor;
    param.handle    = descriptor;
}

auto ink::DynamicDescriptorHeap::bindComputeDescriptor(
    std::uint32_t                          paramIndex,
    std::uint32_t                          offset,
    const D3D12_CONSTANT_BUFFER_VIEW_DESC &desc) noexcept -> void {
    inkAssert(paramIndex < std::size(m_computeTableRange), u"Root parameter index out of range.");

    auto &table = m_computeTableRange[paramIndex];
    inkAssert(offset < table.count, u"Descriptor offset out of range.");

    auto &param          = m_computeDescriptors[static_cast<std::size_t>(table.start) + offset];
    param.cacheType      = CacheType::ConstantBufferView;
    param.constantBuffer = desc;
}

auto ink::DynamicDescriptorHeap::submitGraphicsDescriptors(
    ID3D12GraphicsCommandList *cmdList) noexcept -> void {
    if (m_graphicsRootSignature == nullptr)
        return;

    inkAssert(m_heapType != D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV ||
                  m_graphicsDescriptors.size() <= 1024,
              u"At most 1024 descriptors are support for CBV/SRV/UAV dynamic descriptor heap.");
    inkAssert(m_heapType != D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER ||
                  m_graphicsDescriptors.size() <= 256,
              u"At most 256 descriptors are supported for sampler dynamic descriptor heaps.");

    const auto requiredCount = static_cast<std::uint32_t>(m_graphicsDescriptors.size());

    // Require new descriptor heap if there is no enough space.
    if (requiredCount > m_freeDescriptorCount) {
        if (m_currentHeap != nullptr)
            m_retiredHeaps.push_back(m_currentHeap);

        if (m_heapType == D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV)
            m_currentHeap = m_renderDevice->newDynamicViewHeap();
        else
            m_currentHeap = m_renderDevice->newDynamicSamplerHeap();

        m_currentHandle = DescriptorHandle(m_currentHeap->GetCPUDescriptorHandleForHeapStart(),
                                           m_currentHeap->GetGPUDescriptorHandleForHeapStart());

        auto desc             = m_currentHeap->GetDesc();
        m_freeDescriptorCount = desc.NumDescriptors;
    }

    if (requiredCount != 0)
        cmdList->SetDescriptorHeaps(1, &m_currentHeap);

    // Copy graphics descriptors.
    std::vector<D3D12_CPU_DESCRIPTOR_HANDLE> copyCache;
    std::vector<UINT>                        copyCount;

    copyCache.resize(requiredCount * std::size_t(2));
    copyCount.resize(requiredCount);

    D3D12_CPU_DESCRIPTOR_HANDLE *copySrc = copyCache.data();
    D3D12_CPU_DESCRIPTOR_HANDLE *copyDst = copyCache.data() + requiredCount;

    std::size_t count = 0;
    for (std::uint32_t i = 0; i < std::size(m_graphicsTableRange); ++i) {
        auto &table = m_graphicsTableRange[i];
        if (table.count == 0)
            continue;

        cmdList->SetGraphicsRootDescriptorTable(i, m_currentHandle);

        auto rangeStart = m_graphicsDescriptors.data() + table.start;
        auto rangeEnd   = rangeStart + table.count;
        for (auto param = rangeStart; param != rangeEnd; ++param) {
            switch (param->cacheType) {
            case CacheType::CpuDescriptor:
                copySrc[count]   = param->handle;
                copyDst[count]   = m_currentHandle;
                copyCount[count] = 1;
                count += 1;
                break;

            case CacheType::ConstantBufferView:
                m_device->CreateConstantBufferView(&(param->constantBuffer), m_currentHandle);
                break;

            default:
                break;
            }

            m_currentHandle += m_descriptorSize;
            m_freeDescriptorCount -= 1;
        }
    }

    if (count != 0)
        m_device->CopyDescriptors(static_cast<UINT>(count), copyDst, copyCount.data(),
                                  static_cast<UINT>(count), copySrc, copyCount.data(), m_heapType);
}

auto ink::DynamicDescriptorHeap::submitComputeDescriptors(
    ID3D12GraphicsCommandList *cmdList) noexcept -> void {
    if (m_computeRootSignature == nullptr)
        return;

    inkAssert(m_heapType != D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV ||
                  m_computeDescriptors.size() <= 1024,
              u"At most 1024 descriptors are support for CBV/SRV/UAV dynamic descriptor heap.");
    inkAssert(m_heapType != D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER ||
                  m_computeDescriptors.size() <= 256,
              u"At most 256 descriptors are supported for sampler dynamic descriptor heaps.");

    const auto requiredCount = static_cast<std::uint32_t>(m_computeDescriptors.size());

    // Require new descriptor heap if there is no enough space.
    if (requiredCount > m_freeDescriptorCount) {
        if (m_currentHeap != nullptr)
            m_retiredHeaps.push_back(m_currentHeap);

        if (m_heapType == D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV)
            m_currentHeap = m_renderDevice->newDynamicViewHeap();
        else
            m_currentHeap = m_renderDevice->newDynamicSamplerHeap();

        m_currentHandle = DescriptorHandle(m_currentHeap->GetCPUDescriptorHandleForHeapStart(),
                                           m_currentHeap->GetGPUDescriptorHandleForHeapStart());

        auto desc             = m_currentHeap->GetDesc();
        m_freeDescriptorCount = desc.NumDescriptors;
    }

    if (requiredCount != 0)
        cmdList->SetDescriptorHeaps(1, &m_currentHeap);

    // Copy compute descriptors.
    std::vector<D3D12_CPU_DESCRIPTOR_HANDLE> copyCache;
    std::vector<UINT>                        copyCount;

    copyCache.resize(requiredCount * std::size_t(2));
    copyCount.resize(requiredCount);

    D3D12_CPU_DESCRIPTOR_HANDLE *copySrc = copyCache.data();
    D3D12_CPU_DESCRIPTOR_HANDLE *copyDst = copyCache.data() + requiredCount;

    std::size_t count = 0;
    for (std::uint32_t i = 0; i < std::size(m_computeTableRange); ++i) {
        auto &table = m_computeTableRange[i];
        if (table.count == 0)
            continue;

        cmdList->SetComputeRootDescriptorTable(i, m_currentHandle);

        auto rangeStart = m_computeDescriptors.data() + table.start;
        auto rangeEnd   = rangeStart + table.count;
        for (auto param = rangeStart; param != rangeEnd; ++param) {
            switch (param->cacheType) {
            case CacheType::CpuDescriptor:
                copySrc[count]   = param->handle;
                copyDst[count]   = m_currentHandle;
                copyCount[count] = 1;
                count += 1;
                break;

            case CacheType::ConstantBufferView:
                m_device->CreateConstantBufferView(&(param->constantBuffer), m_currentHandle);
                break;

            default:
                break;
            }

            m_currentHandle += m_descriptorSize;
            m_freeDescriptorCount -= 1;
        }
    }

    if (count != 0)
        m_device->CopyDescriptors(static_cast<UINT>(count), copyDst, copyCount.data(),
                                  static_cast<UINT>(count), copySrc, copyCount.data(), m_heapType);
}

namespace {

inline constexpr const std::uint32_t DEFAULT_PAGE_SIZE = 0x200000; // 2 MiB

enum class DynamicBufferType {
    Upload,
    UnorderedAccess,
};

class DynamicBufferPage final : public GpuResource {
public:
    /// @brief
    ///   Create a new dynamic buffer page.
    /// @note
    ///   Errors are handled with assertions.
    ///
    /// @param bufferType
    ///   Type of this dynamic buffer page.
    /// @param size
    ///   Expected size in byte of this dynamic buffer page.
    DynamicBufferPage(DynamicBufferType bufferType, std::size_t size = DEFAULT_PAGE_SIZE) noexcept;

    /// @brief
    ///   Move constructor of dynamic buffer page.
    ///
    /// @param other
    ///   The dynamic buffer page to be moved from. The moved dynamic buffer page will be
    ///   invalidated.
    DynamicBufferPage(DynamicBufferPage &&other) noexcept;

    /// @brief
    ///   Destroy this dynamic buffer page.
    ~DynamicBufferPage() noexcept override;

    /// @brief
    ///   Map this dynamic buffer page to memory address.
    /// @note
    ///   Only available for uplload buffer page.
    ///
    /// @tparam T
    ///   Type of the pointer to be mapped.
    ///
    /// @return
    ///   Pointer to the mapped memory.
    template <typename T>
    [[nodiscard]]
    auto map() const noexcept -> T * {
        return static_cast<T *>(m_data);
    }

    /// @brief
    ///   Get GPU virtual address to start of this dynamic buffer page.
    ///
    /// @return
    ///   GPU virtual address to start of this dynamic buffer page.
    [[nodiscard]]
    auto gpuAddress() const noexcept -> std::uint64_t {
        return m_gpuAddress;
    }

    /// @brief
    ///   Checks if this is a default dynamic buffer page.
    ///
    /// @return
    ///   A boolean value that indicates whether this is a default dynamic buffer page.
    /// @retval true
    ///   This is a default dynamic buffer page.
    /// @retval false
    ///   This is not a default dynamic buffer page.
    [[nodiscard]]
    auto isDefaultPage() const noexcept -> bool {
        return m_size == DEFAULT_PAGE_SIZE;
    }

    /// @brief
    ///   Checks if this dynamic buffer page is an upload buffer page.
    ///
    /// @return
    ///   A boolean value that indicates whether this is an upload buffer page.
    /// @retval true
    ///   This is an upload buffer page.
    /// @retval false
    ///   This is not an upload buffer page.
    [[nodiscard]]
    auto isUploadBuffer() const noexcept -> bool {
        return m_bufferType == DynamicBufferType::Upload;
    }

private:
    /// @brief
    ///   Type of this dynamic buffer page.
    DynamicBufferType m_bufferType;

    /// @brief
    ///   Size in byte of this dynamic buffer page.
    std::size_t m_size;

    /// @brief
    ///   CPU pointer to start of this dynamic buffer page (upload buffer only).
    void *m_data;

    /// @brief
    ///   GPU virtual address to start of this temp buffer page.
    std::uint64_t m_gpuAddress;
};

DynamicBufferPage::DynamicBufferPage(DynamicBufferType bufferType, std::size_t size) noexcept
    : GpuResource(), m_bufferType(bufferType), m_size(size), m_data(), m_gpuAddress() {
    [[maybe_unused]] HRESULT hr;

    auto &dev = RenderDevice::singleton();
    if (bufferType == DynamicBufferType::Upload) {
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
            /* Width            = */ size,
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

        hr = dev.device()->CreateCommittedResource(&heapProps, D3D12_HEAP_FLAG_NONE, &desc,
                                                   D3D12_RESOURCE_STATE_GENERIC_READ, nullptr,
                                                   IID_PPV_ARGS(m_resource.GetAddressOf()));
        inkAssert(SUCCEEDED(hr),
                  u"Failed to create ID3D12Resource for dynamic buffer page: 0x{:X}.",
                  static_cast<std::uint32_t>(hr));

        m_usageState = D3D12_RESOURCE_STATE_GENERIC_READ;
        m_resource->Map(0, nullptr, &m_data);
        m_gpuAddress = m_resource->GetGPUVirtualAddress();
    } else {
        const D3D12_HEAP_PROPERTIES heapProps{
            /* Type                 = */ D3D12_HEAP_TYPE_DEFAULT,
            /* CPUPageProperty      = */ D3D12_CPU_PAGE_PROPERTY_UNKNOWN,
            /* MemoryPoolPreference = */ D3D12_MEMORY_POOL_UNKNOWN,
            /* CreationNodeMask     = */ 0,
            /* VisibleNodeMask      = */ 0,
        };

        const D3D12_RESOURCE_DESC desc{
            /* Dimension        = */ D3D12_RESOURCE_DIMENSION_BUFFER,
            /* Alignment        = */ 0,
            /* Width            = */ size,
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
            /* Flags  = */ D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS,
        };

        hr =
            dev.device()->CreateCommittedResource(&heapProps, D3D12_HEAP_FLAG_NONE, &desc, state(),
                                                  nullptr, IID_PPV_ARGS(m_resource.GetAddressOf()));
        inkAssert(
            SUCCEEDED(hr),
            u"Failed to create ID3D12Resource for dynamic unordered access buffer page: 0x{:X}.",
            static_cast<std::uint32_t>(hr));

        m_gpuAddress = m_resource->GetGPUVirtualAddress();
    }
}

DynamicBufferPage::DynamicBufferPage(DynamicBufferPage &&other) noexcept
    : GpuResource(static_cast<GpuResource &&>(other)),
      m_bufferType(other.m_bufferType),
      m_size(other.m_size),
      m_data(other.m_data),
      m_gpuAddress(other.m_gpuAddress) {
    other.m_data = nullptr;
}

DynamicBufferPage::~DynamicBufferPage() noexcept {
    if (m_data != nullptr)
        m_resource->Unmap(0, nullptr);
}

class DynamicBufferPageManager {
public:
    /// @brief
    ///   Create a dynamic buffer page manager.
    DynamicBufferPageManager() noexcept;

    /// @brief
    ///   Destroy this dynamic buffer page manager and release all pages.
    ~DynamicBufferPageManager() noexcept;

    /// @brief
    ///   Allocate a new dynamic upload buffer page.
    ///
    /// @param size
    ///   Expected size in byte of this new dynamic buffer page. This value will be aligned up to
    ///   256 bytes.
    ///
    /// @return
    ///   Pointer to the new dynamic upload buffer page.
    [[nodiscard]]
    auto newUploadPage(std::size_t size) noexcept -> DynamicBufferPage *;

    /// @brief
    ///   Allocate a new dynamic unordered access buffer page.
    ///
    /// @param size
    ///   Expected size in byte of this new dynamic buffer page. This value will be aligned up to
    ///   256 bytes.
    ///
    /// @return
    ///   Pointer to the new dynamic access buffer buffer page.
    [[nodiscard]]
    auto newUnorderedAccessPage(std::size_t size) noexcept -> DynamicBufferPage *;

    /// @brief
    ///   Free retired pages.
    ///
    /// @param fenceValue
    ///   Fence value that indicates when the freed pages could be reused.
    /// @param count
    ///   Number of pages to be freed.
    /// @param pages
    ///   An array of dynamic buffer pages to be freed.
    auto freePages(std::uint64_t fenceValue, std::size_t count, DynamicBufferPage **pages) noexcept
        -> void;

    /// @brief
    ///   Get singleton instance of dynamic buffer page manager.
    ///
    /// @return
    ///   Reference to the dynamic buffer page manager singleton.
    [[nodiscard]]
    static auto singleton() noexcept -> DynamicBufferPageManager &;

private:
    /// @brief
    ///   The D3D12 device that is used to sychnorize with GPU.
    RenderDevice &m_renderDevice;

    /// @brief
    ///   Temporary buffer page pool.
    std::stack<DynamicBufferPage> m_pagePool;

    /// @brief
    ///   Mutex to protect page pool.
    mutable std::mutex m_pagePoolMutex;

    /// @brief
    ///   Retired upload pages to be reused.
    std::queue<std::pair<std::uint64_t, DynamicBufferPage *>> m_retiredUploadPages;

    /// @brief
    ///   Mutex to protect retired upload page queue.
    mutable std::mutex m_uploadPageQueueMutex;

    /// @brief
    ///   Retired unordered access pages to be reused.
    std::queue<std::pair<std::uint64_t, DynamicBufferPage *>> m_retiredUnorderedAccessPages;

    /// @brief
    ///   Mutex to protect retired unordered access queue.
    mutable std::mutex m_unorderedAccessQueueMutex;

    /// @brief
    ///   Queue of pages to be deleted.
    std::queue<std::pair<std::uint64_t, DynamicBufferPage *>> m_deletionQueue;

    /// @brief
    ///   Mutex to protect deletion queue.
    mutable std::mutex m_deletionQueueMutex;
};

DynamicBufferPageManager::DynamicBufferPageManager() noexcept
    : m_renderDevice(RenderDevice::singleton()),
      m_pagePool(),
      m_pagePoolMutex(),
      m_retiredUploadPages(),
      m_uploadPageQueueMutex(),
      m_retiredUnorderedAccessPages(),
      m_unorderedAccessQueueMutex(),
      m_deletionQueue(),
      m_deletionQueueMutex() {}

DynamicBufferPageManager::~DynamicBufferPageManager() noexcept {
    m_renderDevice.sync();
    std::lock_guard<std::mutex> lock(m_deletionQueueMutex);
    while (!m_deletionQueue.empty()) {
        delete m_deletionQueue.front().second;
        m_deletionQueue.pop();
    }
}

auto DynamicBufferPageManager::newUploadPage(std::size_t size) noexcept -> DynamicBufferPage * {
    // Align up size.
    size = (size + 0xFF) & ~std::size_t(0xFF);

    if (size <= DEFAULT_PAGE_SIZE) {
        { // Try to get a free page from free page queue.
            std::lock_guard<std::mutex> lock(m_uploadPageQueueMutex);
            if (!m_retiredUploadPages.empty()) {
                auto &front = m_retiredUploadPages.front();
                if (m_renderDevice.isFenceReached(front.first)) {
                    auto *page = front.second;
                    m_retiredUploadPages.pop();
                    return page;
                }
            }
        }

        { // Create a new page.
            DynamicBufferPage           newPage(DynamicBufferType::Upload);
            std::lock_guard<std::mutex> lock(m_pagePoolMutex);
            return std::addressof(m_pagePool.emplace(std::move(newPage)));
        }
    }

    return new DynamicBufferPage(DynamicBufferType::Upload, size);
}

auto DynamicBufferPageManager::newUnorderedAccessPage(std::size_t size) noexcept
    -> DynamicBufferPage * {
    // Align up size.
    size = (size + 0xFF) & ~std::size_t(0xFF);

    if (size <= DEFAULT_PAGE_SIZE) {
        { // Try to get a free page from free page queue.
            std::lock_guard<std::mutex> lock(m_unorderedAccessQueueMutex);
            if (!m_retiredUnorderedAccessPages.empty()) {
                auto &front = m_retiredUnorderedAccessPages.front();
                if (m_renderDevice.isFenceReached(front.first)) {
                    auto *page = front.second;
                    m_retiredUnorderedAccessPages.pop();
                    return page;
                }
            }
        }

        { // Create a new page.
            DynamicBufferPage newPage(DynamicBufferType::UnorderedAccess);

            std::lock_guard<std::mutex> lock(m_pagePoolMutex);
            return std::addressof(m_pagePool.emplace(std::move(newPage)));
        }
    }

    return new DynamicBufferPage(DynamicBufferType::UnorderedAccess, size);
}

auto DynamicBufferPageManager::freePages(std::uint64_t       fenceValue,
                                         std::size_t         count,
                                         DynamicBufferPage **pages) noexcept -> void {
    DynamicBufferPage **pagesEnd = pages + count;

    { // Free default upload pages.
        std::lock_guard<std::mutex> lock(m_uploadPageQueueMutex);
        for (auto *page = pages; page != pagesEnd; ++page) {
            if (!(*page)->isDefaultPage())
                continue;

            if (!(*page)->isUploadBuffer())
                continue;

            m_retiredUploadPages.emplace(fenceValue, *page);
        }
    }

    { // Free unordered access pages.
        std::lock_guard<std::mutex> lock(m_unorderedAccessQueueMutex);
        for (auto *page = pages; page != pagesEnd; ++page) {
            if (!(*page)->isDefaultPage())
                continue;

            if ((*page)->isUploadBuffer())
                continue;

            m_retiredUnorderedAccessPages.emplace(fenceValue, *page);
        }
    }

    { // Free deletion pages.
        std::lock_guard<std::mutex> lock(m_deletionQueueMutex);
        while (!m_deletionQueue.empty()) {
            auto &front = m_deletionQueue.front();
            if (m_renderDevice.isFenceReached(front.first)) {
                delete front.second;
                m_deletionQueue.pop();
            } else {
                break;
            }
        }

        for (auto *page = pages; page != pagesEnd; ++page) {
            if ((*page)->isDefaultPage())
                continue;
            m_deletionQueue.emplace(fenceValue, *page);
        }
    }
}

auto DynamicBufferPageManager::singleton() noexcept -> DynamicBufferPageManager & {
    static DynamicBufferPageManager instance;
    return instance;
}

} // namespace

ink::DynamicBufferAllocator::DynamicBufferAllocator() noexcept
    : m_uploadPage(),
      m_uploadPageOffset(),
      m_unorderedAccessPage(),
      m_unorderedAccessPageOffset(),
      m_retiredPages() {}

ink::DynamicBufferAllocator::~DynamicBufferAllocator() noexcept {
    if (m_uploadPage != nullptr)
        m_retiredPages.push_back(m_uploadPage);
    if (m_unorderedAccessPage != nullptr)
        m_retiredPages.push_back(m_unorderedAccessPage);

    if (!m_retiredPages.empty()) {
        auto &dev        = RenderDevice::singleton();
        auto  fenceValue = dev.signalFence();

        auto &manager = DynamicBufferPageManager::singleton();
        manager.freePages(fenceValue, m_retiredPages.size(),
                          reinterpret_cast<DynamicBufferPage **>(m_retiredPages.data()));
    }
}

auto ink::DynamicBufferAllocator::newUploadBuffer(std::size_t size) noexcept
    -> DynamicBufferAllocation {
    // Align up allocate size.
    size = (size + 0xFF) & ~std::size_t(0xFF);

    auto &manager = DynamicBufferPageManager::singleton();

    // Allocate a single page if size is greater than default page size.
    if (size >= DEFAULT_PAGE_SIZE) {
        DynamicBufferPage *page = manager.newUploadPage(size);
        m_retiredPages.push_back(page);

        return DynamicBufferAllocation{
            /* resource   = */ page,
            /* size       = */ size,
            /* offset     = */ 0,
            /* data       = */ page->map<void>(),
            /* gpuAddress = */ page->gpuAddress(),
        };
    }

    // No enough space in current page, retire current page.
    if (m_uploadPage != nullptr && m_uploadPageOffset + size > DEFAULT_PAGE_SIZE) {
        m_retiredPages.push_back(m_uploadPage);
        m_uploadPage = nullptr;
    }

    // Allocate a new page if current page is null.
    if (m_uploadPage == nullptr) {
        m_uploadPage       = manager.newUploadPage(DEFAULT_PAGE_SIZE);
        m_uploadPageOffset = 0;
    }

    auto *const page = static_cast<DynamicBufferPage *>(m_uploadPage);

    DynamicBufferAllocation allocation{
        /* resource   = */ page,
        /* size       = */ size,
        /* offset     = */ m_uploadPageOffset,
        /* data       = */ page->map<std::uint8_t>() + m_uploadPageOffset,
        /* gpuAddress = */ page->gpuAddress() + m_uploadPageOffset,
    };

    m_uploadPageOffset += size;
    return allocation;
}

auto ink::DynamicBufferAllocator::newUnorderedAccessBuffer(std::size_t size) noexcept
    -> DynamicBufferAllocation {
    // Align up allocate size.
    size = (size + 0xFF) & ~std::size_t(0xFF);

    auto &manager = DynamicBufferPageManager::singleton();

    // Allocate a single page if size is greater than default page size.
    if (size >= DEFAULT_PAGE_SIZE) {
        auto *page = manager.newUnorderedAccessPage(size);
        m_retiredPages.push_back(page);

        return DynamicBufferAllocation{
            /* resource   = */ page,
            /* size       = */ size,
            /* offset     = */ 0,
            /* data       = */ page->map<void>(),
            /* gpuAddress = */ page->gpuAddress(),
        };
    }

    // There is no enough space in current page, retire current page.
    if (m_unorderedAccessPage != nullptr &&
        m_unorderedAccessPageOffset + size > DEFAULT_PAGE_SIZE) {
        m_retiredPages.push_back(m_unorderedAccessPage);
        m_unorderedAccessPage = nullptr;
    }

    // Allocate a new page if current page is null.
    if (m_unorderedAccessPage == nullptr) {
        m_unorderedAccessPage       = manager.newUnorderedAccessPage(DEFAULT_PAGE_SIZE);
        m_unorderedAccessPageOffset = 0;
    }

    auto *const page = static_cast<DynamicBufferPage *>(m_unorderedAccessPage);

    DynamicBufferAllocation allocation{
        /* resource   = */ page,
        /* size       = */ size,
        /* offset     = */ m_unorderedAccessPageOffset,
        /* data       = */ page->map<std::uint8_t>() + m_unorderedAccessPageOffset,
        /* gpuAddress = */ page->gpuAddress() + m_unorderedAccessPageOffset,
    };

    m_unorderedAccessPageOffset += size;
    return allocation;
}

auto ink::DynamicBufferAllocator::reset(std::uint64_t fenceValue) noexcept -> void {
    auto &manager = DynamicBufferPageManager::singleton();

    if (!m_retiredPages.empty()) {
        manager.freePages(fenceValue, m_retiredPages.size(),
                          reinterpret_cast<DynamicBufferPage **>(m_retiredPages.data()));
        m_retiredPages.clear();
    }
}

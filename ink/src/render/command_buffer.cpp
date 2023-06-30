#include "ink/render/command_buffer.h"
#include "ink/core/assert.h"
#include "ink/render/device.h"
#include "ink/render/pipeline.h"

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
            }

            m_currentHandle += m_descriptorSize;
            m_freeDescriptorCount -= 1;
        }
    }

    if (count != 0)
        m_device->CopyDescriptors(static_cast<UINT>(count), copyDst, copyCount.data(),
                                  static_cast<UINT>(count), copySrc, copyCount.data(), m_heapType);
}

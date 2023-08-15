#include "ink/render/pipeline.hpp"
#include "ink/core/exception.hpp"
#include "ink/render/device.hpp"

using namespace ink;
using Microsoft::WRL::ComPtr;

ink::RootSignature::RootSignature(ID3D12Device5 *device, const D3D12_ROOT_SIGNATURE_DESC &desc)
    : m_rootSignature(),
      m_staticSamplerCount(desc.NumStaticSamplers),
      m_tableViewDescriptorCount(),
      m_tableSamplerCount(),
      m_viewTableFlags(),
      m_samplerTableFlags(),
      m_tableSizes() {
    // Serialize the root signature.
    ComPtr<ID3DBlob> signature;
    ComPtr<ID3DBlob> error;

    HRESULT hr = D3D12SerializeRootSignature(&desc, D3D_ROOT_SIGNATURE_VERSION_1_0,
                                             signature.GetAddressOf(), error.GetAddressOf());
    if (FAILED(hr)) {
        std::string msg("Failed to serialize root signature: ");
        msg += static_cast<char *>(error->GetBufferPointer());
        throw RenderAPIException(hr, std::move(msg));
    }

    // Create the root signature.
    hr = device->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(),
                                     IID_PPV_ARGS(m_rootSignature.GetAddressOf()));
    if (FAILED(hr))
        throw RenderAPIException(hr, "Failed to create D3D12 root signature.");

    // Cache root signature metadata.
    for (std::uint32_t i = 0; i < desc.NumParameters; ++i) {
        const auto &param = desc.pParameters[i];
        if (param.ParameterType == D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE) {
            const auto &table      = param.DescriptorTable;
            const auto *rangeBegin = table.pDescriptorRanges;
            const auto *rangeEnd   = rangeBegin + table.NumDescriptorRanges;

            for (auto j = rangeBegin; j != rangeEnd; ++j)
                m_tableSizes[i] += static_cast<std::uint16_t>(j->NumDescriptors);

            if (rangeBegin->RangeType == D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER) {
                m_samplerTableFlags[i] = true;
                m_tableSamplerCount += m_tableSizes[i];
            } else {
                m_viewTableFlags[i] = true;
                m_tableViewDescriptorCount += m_tableSizes[i];
            }
        }
    }
}

ink::RootSignature::RootSignature(const RootSignature &other) noexcept = default;

ink::RootSignature::RootSignature(RootSignature &&other) noexcept = default;

ink::RootSignature::~RootSignature() noexcept = default;

auto ink::RootSignature::operator=(const RootSignature &other) noexcept
    -> RootSignature & = default;

auto ink::RootSignature::operator=(RootSignature &&other) noexcept -> RootSignature & = default;

ink::PipelineState::~PipelineState() noexcept = default;

ink::GraphicsPipelineState::GraphicsPipelineState(ID3D12Device                             *device,
                                                  const D3D12_GRAPHICS_PIPELINE_STATE_DESC &desc)
    : PipelineState(),
      m_renderTargetCount(desc.NumRenderTargets),
      m_renderTargetFormats{
          desc.RTVFormats[0], desc.RTVFormats[1], desc.RTVFormats[2], desc.RTVFormats[3],
          desc.RTVFormats[4], desc.RTVFormats[5], desc.RTVFormats[6], desc.RTVFormats[7],
      },
      m_depthStencilFormat(desc.DSVFormat),
      m_primitiveType(desc.PrimitiveTopologyType),
      m_sampleCount(desc.SampleDesc.Count) {
    HRESULT hr =
        device->CreateGraphicsPipelineState(&desc, IID_PPV_ARGS(m_pipelineState.GetAddressOf()));
    if (FAILED(hr))
        throw RenderAPIException(hr, "Failed to create graphics pipeline state.");
}

ink::GraphicsPipelineState::GraphicsPipelineState() noexcept
    : PipelineState(),
      m_renderTargetCount(),
      m_renderTargetFormats(),
      m_depthStencilFormat(),
      m_primitiveType(),
      m_sampleCount() {}

ink::GraphicsPipelineState::GraphicsPipelineState(const GraphicsPipelineState &other) noexcept =
    default;

ink::GraphicsPipelineState::GraphicsPipelineState(GraphicsPipelineState &&other) noexcept = default;

ink::GraphicsPipelineState::~GraphicsPipelineState() noexcept = default;

auto ink::GraphicsPipelineState::operator=(const GraphicsPipelineState &other) noexcept
    -> GraphicsPipelineState & = default;

auto ink::GraphicsPipelineState::operator=(GraphicsPipelineState &&other) noexcept
    -> GraphicsPipelineState & = default;

ink::ComputePipelineState::ComputePipelineState(ID3D12Device                            *device,
                                                const D3D12_COMPUTE_PIPELINE_STATE_DESC &desc)
    : PipelineState() {
    HRESULT hr =
        device->CreateComputePipelineState(&desc, IID_PPV_ARGS(m_pipelineState.GetAddressOf()));
    if (FAILED(hr))
        throw RenderAPIException(hr, "Failed to create compute pipeline state.");
}

ink::ComputePipelineState::ComputePipelineState() noexcept : PipelineState() {}

ink::ComputePipelineState::ComputePipelineState(const ComputePipelineState &other) noexcept =
    default;

ink::ComputePipelineState::ComputePipelineState(ComputePipelineState &&other) noexcept = default;

ink::ComputePipelineState::~ComputePipelineState() noexcept = default;

auto ink::ComputePipelineState::operator=(const ComputePipelineState &other) noexcept
    -> ComputePipelineState & = default;

auto ink::ComputePipelineState::operator=(ComputePipelineState &&other) noexcept
    -> ComputePipelineState & = default;

ink::DynamicDescriptorHeap::DynamicDescriptorHeap(RenderDevice  &renderDevice,
                                                  ID3D12Device5 *device) noexcept
    : m_renderDevice(&renderDevice),
      m_device(device),
      m_viewSize(device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV)),
      m_samplerSize(device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER)),
      m_graphicsRootSignature(),
      m_computeRootSignature(),
      m_viewHeap(),
      m_samplerHeap(),
      m_currentViewHandle(),
      m_currentSamplerHandle(),
      m_freeViewCount(),
      m_freeSamplerCount(),
      m_retiredViewHeaps(),
      m_retiredSamplerHeaps(),
      m_graphicsDescriptors(),
      m_computeDescriptors(),
      m_graphicsTableRange(),
      m_computeTableRange() {}

ink::DynamicDescriptorHeap::DynamicDescriptorHeap() noexcept
    : m_renderDevice(),
      m_device(),
      m_viewSize(),
      m_samplerSize(),
      m_graphicsRootSignature(),
      m_computeRootSignature(),
      m_viewHeap(),
      m_samplerHeap(),
      m_currentViewHandle(),
      m_currentSamplerHandle(),
      m_freeViewCount(),
      m_freeSamplerCount(),
      m_retiredViewHeaps(),
      m_retiredSamplerHeaps(),
      m_graphicsDescriptors(),
      m_computeDescriptors(),
      m_graphicsTableRange(),
      m_computeTableRange() {}

ink::DynamicDescriptorHeap::DynamicDescriptorHeap(DynamicDescriptorHeap &&other) noexcept
    : m_renderDevice(other.m_renderDevice),
      m_device(other.m_device),
      m_viewSize(other.m_viewSize),
      m_samplerSize(other.m_samplerSize),
      m_graphicsRootSignature(other.m_graphicsRootSignature),
      m_computeRootSignature(other.m_computeRootSignature),
      m_viewHeap(other.m_viewHeap),
      m_samplerHeap(other.m_samplerHeap),
      m_currentViewHandle(other.m_currentViewHandle),
      m_currentSamplerHandle(other.m_currentSamplerHandle),
      m_freeViewCount(other.m_freeViewCount),
      m_freeSamplerCount(other.m_freeSamplerCount),
      m_retiredViewHeaps(std::move(other.m_retiredViewHeaps)),
      m_retiredSamplerHeaps(std::move(other.m_retiredSamplerHeaps)),
      m_graphicsDescriptors(std::move(other.m_graphicsDescriptors)),
      m_computeDescriptors(std::move(other.m_computeDescriptors)),
      m_graphicsTableRange(other.m_graphicsTableRange),
      m_computeTableRange(other.m_computeTableRange) {
    other.m_renderDevice          = nullptr;
    other.m_device                = nullptr;
    other.m_graphicsRootSignature = nullptr;
    other.m_computeRootSignature  = nullptr;
    other.m_viewHeap              = nullptr;
    other.m_samplerHeap           = nullptr;
    other.m_currentViewHandle     = {};
    other.m_currentSamplerHandle  = {};
    other.m_freeViewCount         = 0;
    other.m_freeSamplerCount      = 0;
    other.m_graphicsTableRange    = {};
    other.m_computeTableRange     = {};
}

ink::DynamicDescriptorHeap::~DynamicDescriptorHeap() noexcept {
    if (m_viewHeap != nullptr)
        m_retiredViewHeaps.push_back(m_viewHeap);
    if (!m_retiredViewHeaps.empty())
        m_renderDevice->releaseDynamicViewHeaps(
            m_renderDevice->signalFence(), m_retiredViewHeaps.size(), m_retiredViewHeaps.data());

    if (m_samplerHeap != nullptr)
        m_retiredSamplerHeaps.push_back(m_samplerHeap);
    if (!m_retiredSamplerHeaps.empty())
        m_renderDevice->releaseDynamicSamplerHeaps(m_renderDevice->signalFence(),
                                                   m_retiredSamplerHeaps.size(),
                                                   m_retiredSamplerHeaps.data());
}

auto ink::DynamicDescriptorHeap::operator=(DynamicDescriptorHeap &&other) noexcept
    -> DynamicDescriptorHeap & {
    // Check self move.
    if (this == &other)
        return *this;

    // Destroy current dynamic descriptor heap.
    if (m_viewHeap != nullptr)
        m_retiredViewHeaps.push_back(m_viewHeap);
    if (!m_retiredViewHeaps.empty())
        m_renderDevice->releaseDynamicViewHeaps(
            m_renderDevice->signalFence(), m_retiredViewHeaps.size(), m_retiredViewHeaps.data());

    if (m_samplerHeap != nullptr)
        m_retiredSamplerHeaps.push_back(m_samplerHeap);
    if (!m_retiredSamplerHeaps.empty())
        m_renderDevice->releaseDynamicSamplerHeaps(m_renderDevice->signalFence(),
                                                   m_retiredSamplerHeaps.size(),
                                                   m_retiredSamplerHeaps.data());

    // Move other to this.
    m_renderDevice          = other.m_renderDevice;
    m_device                = other.m_device;
    m_viewSize              = other.m_viewSize;
    m_samplerSize           = other.m_samplerSize;
    m_graphicsRootSignature = other.m_graphicsRootSignature;
    m_computeRootSignature  = other.m_computeRootSignature;
    m_viewHeap              = other.m_viewHeap;
    m_samplerHeap           = other.m_samplerHeap;
    m_currentViewHandle     = other.m_currentViewHandle;
    m_currentSamplerHandle  = other.m_currentSamplerHandle;
    m_freeViewCount         = other.m_freeViewCount;
    m_freeSamplerCount      = other.m_freeSamplerCount;
    m_retiredViewHeaps      = std::move(other.m_retiredViewHeaps);
    m_retiredSamplerHeaps   = std::move(other.m_retiredSamplerHeaps);
    m_graphicsDescriptors   = std::move(other.m_graphicsDescriptors);
    m_computeDescriptors    = std::move(other.m_computeDescriptors);
    m_graphicsTableRange    = other.m_graphicsTableRange;
    m_computeTableRange     = other.m_computeTableRange;

    // Reset other.
    other.m_renderDevice          = nullptr;
    other.m_device                = nullptr;
    other.m_graphicsRootSignature = nullptr;
    other.m_computeRootSignature  = nullptr;
    other.m_viewHeap              = nullptr;
    other.m_samplerHeap           = nullptr;
    other.m_currentViewHandle     = {};
    other.m_currentSamplerHandle  = {};
    other.m_freeViewCount         = 0;
    other.m_freeSamplerCount      = 0;
    other.m_graphicsTableRange    = {};
    other.m_computeTableRange     = {};

    return *this;
}

auto ink::DynamicDescriptorHeap::parseGraphicsRootSignature(const RootSignature &rootSig) noexcept
    -> void {
    if (m_graphicsRootSignature == &rootSig)
        return;

    m_graphicsRootSignature = &rootSig;

    const std::uint32_t viewDescCount = rootSig.tableViewCount();
    const std::uint32_t samplerCount  = rootSig.tableSamplerCount();
    const std::uint32_t descCount     = viewDescCount + samplerCount;

    m_graphicsDescriptors.resize(descCount);
    std::memset(m_graphicsDescriptors.data(), 0, descCount * sizeof(DescriptorCache));

    std::uint32_t offset = 0;
    for (std::uint32_t i = 0; i < m_graphicsTableRange.size(); ++i) {
        std::uint32_t tableSize       = rootSig.tableSize(i);
        m_graphicsTableRange[i].start = static_cast<std::uint16_t>(offset);
        m_graphicsTableRange[i].count = static_cast<std::uint16_t>(tableSize);
        offset += tableSize;
    }
}

auto ink::DynamicDescriptorHeap::parseComputeRootSignature(const RootSignature &rootSig) noexcept
    -> void {
    if (m_computeRootSignature == &rootSig)
        return;

    m_computeRootSignature = &rootSig;

    const std::uint32_t viewDescCount = rootSig.tableViewCount();
    const std::uint32_t samplerCount  = rootSig.tableSamplerCount();
    const std::uint32_t descCount     = viewDescCount + samplerCount;

    m_computeDescriptors.resize(descCount);
    std::memset(m_computeDescriptors.data(), 0, descCount * sizeof(DescriptorCache));

    std::uint32_t offset = 0;
    for (std::uint32_t i = 0; i < m_computeTableRange.size(); ++i) {
        std::uint32_t tableSize      = rootSig.tableSize(i);
        m_computeTableRange[i].start = static_cast<std::uint16_t>(offset);
        m_computeTableRange[i].count = static_cast<std::uint16_t>(tableSize);
        offset += tableSize;
    }
}

auto ink::DynamicDescriptorHeap::bindGraphicsDescriptor(std::uint32_t paramIndex,
                                                        std::uint32_t offset,
                                                        CpuDescriptor descriptor) noexcept -> void {
    assert(paramIndex < m_graphicsTableRange.size() && "Root parameter index out of range.");
    DescriptorTableRange &table = m_graphicsTableRange[paramIndex];
    assert(offset < table.count && "Descriptor offset out of range.");

    DescriptorCache &param = m_graphicsDescriptors[static_cast<std::size_t>(table.start) + offset];
    param.cacheType        = CacheType::CpuDescriptor;
    param.handle           = descriptor;
}

auto ink::DynamicDescriptorHeap::bindGraphicsDescriptor(
    std::uint32_t                          paramIndex,
    std::uint32_t                          offset,
    const D3D12_CONSTANT_BUFFER_VIEW_DESC &desc) noexcept -> void {
    assert(paramIndex < m_graphicsTableRange.size() && "Root parameter index out of range.");
    DescriptorTableRange &table = m_graphicsTableRange[paramIndex];
    assert(offset < table.count && "Descriptor offset out of range.");

    DescriptorCache &param = m_graphicsDescriptors[static_cast<std::size_t>(table.start) + offset];
    param.cacheType        = CacheType::ConstantBufferView;
    param.constantBuffer   = desc;
}

auto ink::DynamicDescriptorHeap::bindComputeDescriptor(std::uint32_t paramIndex,
                                                       std::uint32_t offset,
                                                       CpuDescriptor descriptor) noexcept -> void {
    assert(paramIndex < m_computeTableRange.size() && "Root parameter index out of range.");
    DescriptorTableRange &table = m_computeTableRange[paramIndex];
    assert(offset < table.count && "Descriptor offset out of range.");

    DescriptorCache &param = m_computeDescriptors[static_cast<std::size_t>(table.start) + offset];
    param.cacheType        = CacheType::CpuDescriptor;
    param.handle           = descriptor;
}

auto ink::DynamicDescriptorHeap::bindComputeDescriptor(
    std::uint32_t                          paramIndex,
    std::uint32_t                          offset,
    const D3D12_CONSTANT_BUFFER_VIEW_DESC &desc) noexcept -> void {
    assert(paramIndex < m_computeTableRange.size() && "Root parameter index out of range.");
    DescriptorTableRange &table = m_computeTableRange[paramIndex];
    assert(offset < table.count && "Descriptor offset out of range.");

    DescriptorCache &param = m_computeDescriptors[static_cast<std::size_t>(table.start) + offset];
    param.cacheType        = CacheType::ConstantBufferView;
    param.constantBuffer   = desc;
}

auto ink::DynamicDescriptorHeap::submitGraphicsDescriptors(ID3D12GraphicsCommandList *cmdList)
    -> void {
    if (m_graphicsRootSignature == nullptr)
        return;

    const std::uint32_t requiredViewCount    = m_graphicsRootSignature->tableViewCount();
    const std::uint32_t requiredSamplerCount = m_graphicsRootSignature->tableSamplerCount();

    assert(requiredViewCount <= 1024);
    assert(requiredSamplerCount <= 256);

    // Require new descriptor heap if there is no enough space.
    if (requiredViewCount > m_freeViewCount) {
        if (m_viewHeap != nullptr) {
            m_retiredViewHeaps.push_back(m_viewHeap);
            m_viewHeap = nullptr;
        }

        m_viewHeap          = m_renderDevice->acquireDynamicViewHeap();
        m_currentViewHandle = {m_viewHeap->GetCPUDescriptorHandleForHeapStart(),
                               m_viewHeap->GetGPUDescriptorHandleForHeapStart()};

        m_freeViewCount = m_viewHeap->GetDesc().NumDescriptors;
    }

    if (requiredSamplerCount > m_freeSamplerCount) {
        if (m_samplerHeap != nullptr) {
            m_retiredSamplerHeaps.push_back(m_samplerHeap);
            m_samplerHeap = nullptr;
        }

        m_samplerHeap          = m_renderDevice->acquireDynamicSamplerHeap();
        m_currentSamplerHandle = {m_samplerHeap->GetCPUDescriptorHandleForHeapStart(),
                                  m_samplerHeap->GetGPUDescriptorHandleForHeapStart()};

        m_freeSamplerCount = m_samplerHeap->GetDesc().NumDescriptors;
    }

    { // Set descriptor heaps.
        std::array<ID3D12DescriptorHeap *, 2> heaps;
        std::uint32_t                         count = 0;

        if (requiredViewCount > 0)
            heaps[count++] = m_viewHeap;
        if (requiredSamplerCount > 0)
            heaps[count++] = m_samplerHeap;

        cmdList->SetDescriptorHeaps(count, heaps.data());
    }

    // Copy graphics descriptors.
    std::vector<D3D12_CPU_DESCRIPTOR_HANDLE> copyCache;
    std::vector<UINT>                        copyCount;

    const std::uint32_t requiredCount = requiredViewCount + requiredSamplerCount;

    copyCache.resize(requiredCount * static_cast<std::size_t>(2));
    copyCount.resize(requiredCount);

    D3D12_CPU_DESCRIPTOR_HANDLE *viewCopySrc    = copyCache.data();
    D3D12_CPU_DESCRIPTOR_HANDLE *samplerCopySrc = viewCopySrc + requiredViewCount;
    D3D12_CPU_DESCRIPTOR_HANDLE *viewCopyDst    = copyCache.data() + requiredCount;
    D3D12_CPU_DESCRIPTOR_HANDLE *samplerCopyDst = viewCopyDst + requiredViewCount;

    UINT *viewCopyCount    = copyCount.data();
    UINT *samplerCopyCount = viewCopyCount + requiredViewCount;

    std::uint32_t viewCount    = 0;
    std::uint32_t samplerCount = 0;
    for (std::uint32_t i = 0; i < m_graphicsTableRange.size(); ++i) {
        DescriptorTableRange &table = m_graphicsTableRange[i];
        if (table.count == 0)
            continue;

        if (m_graphicsRootSignature->isViewTable(i)) {
            cmdList->SetGraphicsRootDescriptorTable(i, m_currentViewHandle);

            auto *rangeStart = m_graphicsDescriptors.data() + table.start;
            auto *rangeEnd   = rangeStart + table.count;
            for (auto *param = rangeStart; param != rangeEnd; ++param) {
                switch (param->cacheType) {
                case CacheType::CpuDescriptor:
                    viewCopySrc[viewCount]   = param->handle;
                    viewCopyDst[viewCount]   = m_currentViewHandle;
                    viewCopyCount[viewCount] = 1;
                    viewCount += 1;
                    break;

                case CacheType::ConstantBufferView:
                    m_device->CreateConstantBufferView(&(param->constantBuffer),
                                                       m_currentViewHandle);
                    break;

                default:
                    break;
                }

                m_currentViewHandle += m_viewSize;
                m_freeViewCount -= 1;
            }
        } else {
            assert(m_graphicsRootSignature->isSamplerTable(i));
            cmdList->SetGraphicsRootDescriptorTable(i, m_currentSamplerHandle);

            auto *rangeStart = m_graphicsDescriptors.data() + table.start;
            auto *rangeEnd   = rangeStart + table.count;
            for (auto *param = rangeStart; param != rangeEnd; ++param) {
                assert(param->cacheType != CacheType::ConstantBufferView);
                if (param->cacheType == CacheType::CpuDescriptor) {
                    samplerCopySrc[samplerCount]   = param->handle;
                    samplerCopyDst[samplerCount]   = m_currentSamplerHandle;
                    samplerCopyCount[samplerCount] = 1;
                    samplerCount += 1;
                }

                m_currentSamplerHandle += m_samplerSize;
                m_freeSamplerCount -= 1;
            }
        }
    }

    if (viewCount > 0)
        m_device->CopyDescriptors(viewCount, viewCopyDst, viewCopyCount, viewCount, viewCopySrc,
                                  viewCopyCount, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

    if (samplerCount > 0)
        m_device->CopyDescriptors(samplerCount, samplerCopyDst, samplerCopyCount, samplerCount,
                                  samplerCopySrc, samplerCopyCount,
                                  D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER);
}

auto ink::DynamicDescriptorHeap::submitComputeDescriptors(ID3D12GraphicsCommandList *cmdList)
    -> void {
    if (m_computeRootSignature == nullptr)
        return;

    const std::uint32_t requiredViewCount    = m_computeRootSignature->tableViewCount();
    const std::uint32_t requiredSamplerCount = m_computeRootSignature->tableSamplerCount();

    assert(requiredViewCount <= 1024);
    assert(requiredSamplerCount <= 256);

    // Require new descriptor heap if there is no enough space.
    if (requiredViewCount > m_freeViewCount) {
        if (m_viewHeap != nullptr) {
            m_retiredViewHeaps.push_back(m_viewHeap);
            m_viewHeap = nullptr;
        }

        m_viewHeap          = m_renderDevice->acquireDynamicViewHeap();
        m_currentViewHandle = {m_viewHeap->GetCPUDescriptorHandleForHeapStart(),
                               m_viewHeap->GetGPUDescriptorHandleForHeapStart()};

        m_freeViewCount = m_viewHeap->GetDesc().NumDescriptors;
    }

    if (requiredSamplerCount > m_freeSamplerCount) {
        if (m_samplerHeap != nullptr) {
            m_retiredSamplerHeaps.push_back(m_samplerHeap);
            m_samplerHeap = nullptr;
        }

        m_samplerHeap          = m_renderDevice->acquireDynamicSamplerHeap();
        m_currentSamplerHandle = {m_samplerHeap->GetCPUDescriptorHandleForHeapStart(),
                                  m_samplerHeap->GetGPUDescriptorHandleForHeapStart()};

        m_freeSamplerCount = m_samplerHeap->GetDesc().NumDescriptors;
    }

    { // Set descriptor heaps.
        std::array<ID3D12DescriptorHeap *, 2> heaps;
        std::uint32_t                         count = 0;

        if (requiredViewCount > 0)
            heaps[count++] = m_viewHeap;
        if (requiredSamplerCount > 0)
            heaps[count++] = m_samplerHeap;

        cmdList->SetDescriptorHeaps(count, heaps.data());
    }

    // Copy compute descriptors.
    std::vector<D3D12_CPU_DESCRIPTOR_HANDLE> copyCache;
    std::vector<UINT>                        copyCount;

    const std::uint32_t requiredCount = requiredViewCount + requiredSamplerCount;

    copyCache.resize(requiredCount * static_cast<std::size_t>(2));
    copyCount.resize(requiredCount);

    D3D12_CPU_DESCRIPTOR_HANDLE *viewCopySrc    = copyCache.data();
    D3D12_CPU_DESCRIPTOR_HANDLE *samplerCopySrc = viewCopySrc + requiredViewCount;
    D3D12_CPU_DESCRIPTOR_HANDLE *viewCopyDst    = copyCache.data() + requiredCount;
    D3D12_CPU_DESCRIPTOR_HANDLE *samplerCopyDst = viewCopyDst + requiredViewCount;

    UINT *viewCopyCount    = copyCount.data();
    UINT *samplerCopyCount = viewCopyCount + requiredViewCount;

    std::uint32_t viewCount    = 0;
    std::uint32_t samplerCount = 0;
    for (std::uint32_t i = 0; i < m_computeTableRange.size(); ++i) {
        DescriptorTableRange &table = m_computeTableRange[i];
        if (table.count == 0)
            continue;

        if (m_computeRootSignature->isViewTable(i)) {
            cmdList->SetGraphicsRootDescriptorTable(i, m_currentViewHandle);

            auto *rangeStart = m_computeDescriptors.data() + table.start;
            auto *rangeEnd   = rangeStart + table.count;
            for (auto *param = rangeStart; param != rangeEnd; ++param) {
                switch (param->cacheType) {
                case CacheType::CpuDescriptor:
                    viewCopySrc[viewCount]   = param->handle;
                    viewCopyDst[viewCount]   = m_currentViewHandle;
                    viewCopyCount[viewCount] = 1;
                    viewCount += 1;
                    break;

                case CacheType::ConstantBufferView:
                    m_device->CreateConstantBufferView(&(param->constantBuffer),
                                                       m_currentViewHandle);
                    break;

                default:
                    break;
                }

                m_currentViewHandle += m_viewSize;
                m_freeViewCount -= 1;
            }
        } else {
            assert(m_computeRootSignature->isSamplerTable(i));
            cmdList->SetGraphicsRootDescriptorTable(i, m_currentSamplerHandle);

            auto *rangeStart = m_computeDescriptors.data() + table.start;
            auto *rangeEnd   = rangeStart + table.count;
            for (auto *param = rangeStart; param != rangeEnd; ++param) {
                assert(param->cacheType != CacheType::ConstantBufferView);
                if (param->cacheType == CacheType::CpuDescriptor) {
                    samplerCopySrc[samplerCount]   = param->handle;
                    samplerCopyDst[samplerCount]   = m_currentSamplerHandle;
                    samplerCopyCount[samplerCount] = 1;
                    samplerCount += 1;
                }

                m_currentSamplerHandle += m_samplerSize;
                m_freeSamplerCount -= 1;
            }
        }
    }

    if (viewCount > 0)
        m_device->CopyDescriptors(viewCount, viewCopyDst, viewCopyCount, viewCount, viewCopySrc,
                                  viewCopyCount, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

    if (samplerCount > 0)
        m_device->CopyDescriptors(samplerCount, samplerCopyDst, samplerCopyCount, samplerCount,
                                  samplerCopySrc, samplerCopyCount,
                                  D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER);
}

auto ink::DynamicDescriptorHeap::reset(std::uint64_t fenceValue) noexcept -> void {
    if (!m_retiredViewHeaps.empty()) {
        m_renderDevice->releaseDynamicViewHeaps(fenceValue, m_retiredViewHeaps.size(),
                                                m_retiredViewHeaps.data());
        m_retiredViewHeaps.clear();
    }

    if (!m_retiredSamplerHeaps.empty()) {
        m_renderDevice->releaseDynamicSamplerHeaps(fenceValue, m_retiredSamplerHeaps.size(),
                                                   m_retiredSamplerHeaps.data());
        m_retiredSamplerHeaps.clear();
    }
}

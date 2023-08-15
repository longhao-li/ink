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

ink::CommandBuffer::CommandBuffer(RenderDevice &renderDevice, ID3D12Device5 *device)
    : m_renderDevice(&renderDevice),
      m_cmdList(),
      m_allocator(renderDevice.acquireCommandAllocator()),
      m_lastSubmitFence(),
      m_bufferAllocator(renderDevice),
      m_graphicsRootSignature(),
      m_computeRootSignature(),
      m_dynamicDescriptorHeap(renderDevice, device),
      m_renderPass() {
    HRESULT hr = device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_allocator, nullptr,
                                           IID_PPV_ARGS(m_cmdList.GetAddressOf()));
    if (FAILED(hr)) {
        // Avoid resource leak.
        renderDevice.releaseCommandAllocator(0, m_allocator);
        throw RenderAPIException(hr, "Failed to create new command buffer.");
    }
}

ink::CommandBuffer::CommandBuffer() noexcept
    : m_renderDevice(),
      m_cmdList(),
      m_allocator(),
      m_lastSubmitFence(),
      m_bufferAllocator(),
      m_graphicsRootSignature(),
      m_computeRootSignature(),
      m_dynamicDescriptorHeap(),
      m_renderPass() {}

ink::CommandBuffer::CommandBuffer(CommandBuffer &&other) noexcept
    : m_renderDevice(other.m_renderDevice),
      m_cmdList(std::move(other.m_cmdList)),
      m_allocator(other.m_allocator),
      m_lastSubmitFence(other.m_lastSubmitFence),
      m_bufferAllocator(std::move(other.m_bufferAllocator)),
      m_graphicsRootSignature(other.m_graphicsRootSignature),
      m_computeRootSignature(other.m_computeRootSignature),
      m_dynamicDescriptorHeap(std::move(other.m_dynamicDescriptorHeap)),
      m_renderPass(other.m_renderPass) {
    other.m_allocator             = nullptr;
    other.m_graphicsRootSignature = nullptr;
    other.m_computeRootSignature  = nullptr;
}

ink::CommandBuffer::~CommandBuffer() noexcept {
    if (m_allocator != nullptr)
        m_renderDevice->releaseCommandAllocator(m_lastSubmitFence, m_allocator);
}

auto ink::CommandBuffer::operator=(CommandBuffer &&other) noexcept -> CommandBuffer & {
    if (this == &other)
        return *this;

    if (m_allocator != nullptr)
        m_renderDevice->releaseCommandAllocator(m_lastSubmitFence, m_allocator);

    m_renderDevice          = other.m_renderDevice;
    m_cmdList               = std::move(other.m_cmdList);
    m_allocator             = other.m_allocator;
    m_lastSubmitFence       = other.m_lastSubmitFence;
    m_bufferAllocator       = std::move(other.m_bufferAllocator);
    m_graphicsRootSignature = other.m_graphicsRootSignature;
    m_computeRootSignature  = other.m_computeRootSignature;
    m_dynamicDescriptorHeap = std::move(other.m_dynamicDescriptorHeap);
    m_renderPass            = other.m_renderPass;

    other.m_allocator             = nullptr;
    other.m_graphicsRootSignature = nullptr;
    other.m_computeRootSignature  = nullptr;

    return *this;
}

auto ink::CommandBuffer::submit() -> void {
    m_cmdList->Close();

    { // Submit command list.
        ID3D12CommandList *list = m_cmdList.Get();
        m_renderDevice->m_commandQueue->ExecuteCommandLists(1, &list);
    }

    // Acquire fence value.
    m_lastSubmitFence = m_renderDevice->signalFence();

    // Clean up temporary buffer allocator.
    m_bufferAllocator.reset(m_lastSubmitFence);

    // Clean up root signature.
    m_dynamicDescriptorHeap.reset(m_lastSubmitFence);
    m_graphicsRootSignature = nullptr;
    m_computeRootSignature  = nullptr;

    // Reset command allocator.
    m_renderDevice->releaseCommandAllocator(m_lastSubmitFence, m_allocator);
    m_allocator = nullptr; // Make reset() available if exception is thrown.
    m_allocator = m_renderDevice->acquireCommandAllocator();
    m_cmdList->Reset(m_allocator, nullptr);
}

auto ink::CommandBuffer::reset() -> void {
    m_cmdList->Close();

    // Clean up temporary buffer allocaator.
    m_bufferAllocator.reset(m_lastSubmitFence);

    // Clean up root signature.
    m_dynamicDescriptorHeap.reset(m_lastSubmitFence);
    m_graphicsRootSignature = nullptr;
    m_computeRootSignature  = nullptr;

    if (m_allocator == nullptr)
        m_allocator = m_renderDevice->acquireCommandAllocator();
    else
        m_allocator->Reset();

    m_cmdList->Reset(m_allocator, nullptr);
}

auto ink::CommandBuffer::waitForComplete() const -> void {
    m_renderDevice->sync(m_lastSubmitFence);
}

auto ink::CommandBuffer::transition(GpuResource &resource, D3D12_RESOURCE_STATES newState) noexcept
    -> void {
    if (resource.state() == newState)
        return;

    std::array<D3D12_RESOURCE_BARRIER, 2> barriers;
    std::uint32_t                         barrierCount = 1U;

    const auto oldState = resource.state();

    barriers[0].Type                   = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
    barriers[0].Flags                  = D3D12_RESOURCE_BARRIER_FLAG_NONE;
    barriers[0].Transition.pResource   = resource.m_resource.Get();
    barriers[0].Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
    barriers[0].Transition.StateBefore = oldState;
    barriers[0].Transition.StateAfter  = newState;

    resource.m_usageState = newState;

    if (newState == D3D12_RESOURCE_STATE_UNORDERED_ACCESS) {
        barriers[1].Type          = D3D12_RESOURCE_BARRIER_TYPE_UAV;
        barriers[1].Flags         = D3D12_RESOURCE_BARRIER_FLAG_NONE;
        barriers[1].UAV.pResource = resource.m_resource.Get();

        barrierCount = 2U;
    }

    m_cmdList->ResourceBarrier(barrierCount, barriers.data());
}

// MSVC analyzer seems to incorrectly report the C28020 warning.
#if !defined(__clang__) && defined(_MSC_VER)
#    pragma warning(push)
#    pragma warning(disable : 28020)
#endif

auto ink::CommandBuffer::beginRenderPass(const RenderPass &renderPass) noexcept -> void {
    assert(renderPass.renderTargetCount <= 8);

    { // Transition states.
        std::array<D3D12_RESOURCE_BARRIER, 18> barriers;
        std::uint32_t                          count = 0;

        for (std::uint32_t i = 0; i < renderPass.renderTargetCount; ++i) {
            const auto &info = renderPass.renderTargets[i];
            if (info.renderTarget->state() != info.stateBefore) {
                const auto oldState = info.renderTarget->state();
                const auto newState = info.stateBefore;

                auto &barrier                  = barriers[count++];
                barrier.Type                   = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
                barrier.Flags                  = D3D12_RESOURCE_BARRIER_FLAG_NONE;
                barrier.Transition.pResource   = info.renderTarget->m_resource.Get();
                barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
                barrier.Transition.StateBefore = oldState;
                barrier.Transition.StateAfter  = newState;

                info.renderTarget->m_usageState = newState;

                if ((newState & D3D12_RESOURCE_STATE_UNORDERED_ACCESS) &&
                    !(oldState & D3D12_RESOURCE_STATE_UNORDERED_ACCESS)) {
                    auto &uavBarrier         = barriers[count++];
                    uavBarrier.Type          = D3D12_RESOURCE_BARRIER_TYPE_UAV;
                    uavBarrier.Flags         = D3D12_RESOURCE_BARRIER_FLAG_NONE;
                    uavBarrier.UAV.pResource = info.renderTarget->m_resource.Get();
                }
            }
        }

        if (renderPass.depthTarget.depthTarget &&
            renderPass.depthTarget.depthTarget->state() != renderPass.depthTarget.stateBefore) {
            const auto oldState = renderPass.depthTarget.depthTarget->state();
            const auto newState = renderPass.depthTarget.stateBefore;

            auto &barrier                  = barriers[count++];
            barrier.Type                   = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
            barrier.Flags                  = D3D12_RESOURCE_BARRIER_FLAG_NONE;
            barrier.Transition.pResource   = renderPass.depthTarget.depthTarget->m_resource.Get();
            barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
            barrier.Transition.StateBefore = oldState;
            barrier.Transition.StateAfter  = newState;

            renderPass.depthTarget.depthTarget->m_usageState = newState;

            if ((newState & D3D12_RESOURCE_STATE_UNORDERED_ACCESS) &&
                !(oldState & D3D12_RESOURCE_STATE_UNORDERED_ACCESS)) {
                auto &uavBarrier         = barriers[count++];
                uavBarrier.Type          = D3D12_RESOURCE_BARRIER_TYPE_UAV;
                uavBarrier.Flags         = D3D12_RESOURCE_BARRIER_FLAG_NONE;
                uavBarrier.UAV.pResource = renderPass.depthTarget.depthTarget->m_resource.Get();
            }
        }

        if (count > 0)
            m_cmdList->ResourceBarrier(count, barriers.data());
    }

    // Begin render pass.
    std::array<D3D12_RENDER_PASS_RENDER_TARGET_DESC, 8> renderTargets;
    D3D12_RENDER_PASS_DEPTH_STENCIL_DESC                depthTarget;
    D3D12_RENDER_PASS_DEPTH_STENCIL_DESC               *depthTargetPtr = nullptr;

    for (std::uint32_t i = 0; i < renderPass.renderTargetCount; ++i) {
        const auto &info               = renderPass.renderTargets[i];
        renderTargets[i].cpuDescriptor = info.renderTarget->renderTargetView();

        // Begin access info.
        renderTargets[i].BeginningAccess.Type =
            static_cast<D3D12_RENDER_PASS_BEGINNING_ACCESS_TYPE>(info.loadAction);
        renderTargets[i].BeginningAccess.Clear.ClearValue.Format = info.renderTarget->pixelFormat();

        const Color &clearColor = info.renderTarget->clearColor();
        renderTargets[i].BeginningAccess.Clear.ClearValue.Color[0] = clearColor.red;
        renderTargets[i].BeginningAccess.Clear.ClearValue.Color[1] = clearColor.green;
        renderTargets[i].BeginningAccess.Clear.ClearValue.Color[2] = clearColor.blue;
        renderTargets[i].BeginningAccess.Clear.ClearValue.Color[3] = clearColor.alpha;

        // End access info.
        renderTargets[i].EndingAccess.Type =
            static_cast<D3D12_RENDER_PASS_ENDING_ACCESS_TYPE>(info.storeAction);
        renderTargets[i].EndingAccess.Resolve = {};
    }

    if (renderPass.depthTarget.depthTarget != nullptr) { // Set depth target info.
        const auto &info          = renderPass.depthTarget;
        depthTarget.cpuDescriptor = info.depthTarget->depthStencilView();

        // Begin access info.
        depthTarget.DepthBeginningAccess.Type =
            static_cast<D3D12_RENDER_PASS_BEGINNING_ACCESS_TYPE>(info.depthLoadAction);
        depthTarget.DepthBeginningAccess.Clear.ClearValue.Format = info.depthTarget->pixelFormat();
        depthTarget.DepthBeginningAccess.Clear.ClearValue.DepthStencil.Depth =
            info.depthTarget->clearDepth();
        depthTarget.DepthBeginningAccess.Clear.ClearValue.DepthStencil.Stencil =
            info.depthTarget->clearStencil();
        depthTarget.StencilBeginningAccess.Type =
            static_cast<D3D12_RENDER_PASS_BEGINNING_ACCESS_TYPE>(info.stencilLoadAction);
        depthTarget.StencilBeginningAccess.Clear.ClearValue.Format =
            info.depthTarget->pixelFormat();
        depthTarget.StencilBeginningAccess.Clear.ClearValue.DepthStencil.Depth =
            info.depthTarget->clearDepth();
        depthTarget.StencilBeginningAccess.Clear.ClearValue.DepthStencil.Stencil =
            info.depthTarget->clearStencil();

        // End access info.
        depthTarget.DepthEndingAccess.Type =
            static_cast<D3D12_RENDER_PASS_ENDING_ACCESS_TYPE>(info.depthStoreAction);
        depthTarget.DepthEndingAccess.Resolve = {};
        depthTarget.StencilEndingAccess.Type =
            static_cast<D3D12_RENDER_PASS_ENDING_ACCESS_TYPE>(info.stencilStoreAction);
        depthTarget.StencilEndingAccess.Resolve = {};

        depthTargetPtr = &depthTarget;
    }

    m_cmdList->BeginRenderPass(renderPass.renderTargetCount, renderTargets.data(), depthTargetPtr,
                               D3D12_RENDER_PASS_FLAG_NONE);

    m_renderPass = renderPass;
}

auto ink::CommandBuffer::endRenderPass() noexcept -> void {
    m_cmdList->EndRenderPass();

    // Transition resource states.
    std::array<D3D12_RESOURCE_BARRIER, 18> barriers;
    std::uint32_t                          count = 0;

    for (std::uint32_t i = 0; i < m_renderPass.renderTargetCount; ++i) {
        const auto oldState = m_renderPass.renderTargets[i].renderTarget->state();
        const auto newState = m_renderPass.renderTargets[i].stateAfter;

        if (oldState == newState)
            continue;

        auto &barrier                = barriers[count++];
        barrier.Type                 = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
        barrier.Flags                = D3D12_RESOURCE_BARRIER_FLAG_NONE;
        barrier.Transition.pResource = m_renderPass.renderTargets[i].renderTarget->m_resource.Get();
        barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
        barrier.Transition.StateBefore = oldState;
        barrier.Transition.StateAfter  = newState;

        m_renderPass.renderTargets[i].renderTarget->m_usageState = newState;

        if ((newState & D3D12_RESOURCE_STATE_UNORDERED_ACCESS) &&
            !(oldState & D3D12_RESOURCE_STATE_UNORDERED_ACCESS)) {
            auto &uavBarrier         = barriers[count++];
            uavBarrier.Type          = D3D12_RESOURCE_BARRIER_TYPE_UAV;
            uavBarrier.Flags         = D3D12_RESOURCE_BARRIER_FLAG_NONE;
            uavBarrier.UAV.pResource = m_renderPass.renderTargets[i].renderTarget->m_resource.Get();
        }
    }

    // Transition depth target.
    if (m_renderPass.depthTarget.depthTarget != nullptr) {
        const auto oldState = m_renderPass.depthTarget.depthTarget->state();
        const auto newState = m_renderPass.depthTarget.stateAfter;

        if (oldState != newState) {
            auto &barrier                  = barriers[count++];
            barrier.Type                   = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
            barrier.Flags                  = D3D12_RESOURCE_BARRIER_FLAG_NONE;
            barrier.Transition.pResource   = m_renderPass.depthTarget.depthTarget->m_resource.Get();
            barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
            barrier.Transition.StateBefore = oldState;
            barrier.Transition.StateAfter  = newState;

            m_renderPass.depthTarget.depthTarget->m_usageState = newState;

            if ((newState & D3D12_RESOURCE_STATE_UNORDERED_ACCESS) &&
                !(oldState & D3D12_RESOURCE_STATE_UNORDERED_ACCESS)) {
                auto &uavBarrier         = barriers[count++];
                uavBarrier.Type          = D3D12_RESOURCE_BARRIER_TYPE_UAV;
                uavBarrier.Flags         = D3D12_RESOURCE_BARRIER_FLAG_NONE;
                uavBarrier.UAV.pResource = m_renderPass.depthTarget.depthTarget->m_resource.Get();
            }
        }
    }

    if (count > 0)
        m_cmdList->ResourceBarrier(count, barriers.data());
}

#if !defined(__clang__) && defined(_MSC_VER)
#    pragma warning(pop)
#endif

auto ink::CommandBuffer::copy(GpuResource &src, GpuResource &dst) noexcept -> void {
    { // Transition resource state.
        std::array<D3D12_RESOURCE_BARRIER, 2> barriers;
        std::uint32_t                         count = 0;

        if (!(src.state() & D3D12_RESOURCE_STATE_COPY_SOURCE)) {
            auto &barrier                  = barriers[count++];
            barrier.Type                   = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
            barrier.Flags                  = D3D12_RESOURCE_BARRIER_FLAG_NONE;
            barrier.Transition.pResource   = src.m_resource.Get();
            barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
            barrier.Transition.StateBefore = src.state();
            barrier.Transition.StateAfter  = D3D12_RESOURCE_STATE_COPY_SOURCE;

            src.m_usageState = D3D12_RESOURCE_STATE_COPY_SOURCE;
        }

        if (!(dst.state() & D3D12_RESOURCE_STATE_COPY_DEST)) {
            auto &barrier                  = barriers[count++];
            barrier.Type                   = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
            barrier.Flags                  = D3D12_RESOURCE_BARRIER_FLAG_NONE;
            barrier.Transition.pResource   = dst.m_resource.Get();
            barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
            barrier.Transition.StateBefore = dst.state();
            barrier.Transition.StateAfter  = D3D12_RESOURCE_STATE_COPY_DEST;

            dst.m_usageState = D3D12_RESOURCE_STATE_COPY_DEST;
        }

        if (count > 0)
            m_cmdList->ResourceBarrier(count, barriers.data());
    }

    m_cmdList->CopyResource(dst.m_resource.Get(), src.m_resource.Get());
}

auto ink::CommandBuffer::copyBuffer(GpuResource &src,
                                    std::size_t  srcOffset,
                                    GpuResource &dst,
                                    std::size_t  dstOffset,
                                    std::size_t  size) noexcept -> void {
    { // Transition resource state.
        std::array<D3D12_RESOURCE_BARRIER, 2> barriers;
        std::uint32_t                         count = 0;

        if (!(src.state() & D3D12_RESOURCE_STATE_COPY_SOURCE)) {
            auto &barrier                  = barriers[count++];
            barrier.Type                   = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
            barrier.Flags                  = D3D12_RESOURCE_BARRIER_FLAG_NONE;
            barrier.Transition.pResource   = src.m_resource.Get();
            barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
            barrier.Transition.StateBefore = src.state();
            barrier.Transition.StateAfter  = D3D12_RESOURCE_STATE_COPY_SOURCE;

            src.m_usageState = D3D12_RESOURCE_STATE_COPY_SOURCE;
        }

        if (!(dst.state() & D3D12_RESOURCE_STATE_COPY_DEST)) {
            auto &barrier                  = barriers[count++];
            barrier.Type                   = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
            barrier.Flags                  = D3D12_RESOURCE_BARRIER_FLAG_NONE;
            barrier.Transition.pResource   = dst.m_resource.Get();
            barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
            barrier.Transition.StateBefore = dst.state();
            barrier.Transition.StateAfter  = D3D12_RESOURCE_STATE_COPY_DEST;

            dst.m_usageState = D3D12_RESOURCE_STATE_COPY_DEST;
        }

        if (count > 0)
            m_cmdList->ResourceBarrier(count, barriers.data());
    }

    m_cmdList->CopyBufferRegion(dst.m_resource.Get(), dstOffset, src.m_resource.Get(), srcOffset,
                                size);
}

auto ink::CommandBuffer::copyBuffer(const void  *src,
                                    GpuResource &dst,
                                    std::size_t  dstOffset,
                                    std::size_t  size) -> void {
    DynamicBufferAllocation allocation(m_bufferAllocator.allocate(size));
    std::memcpy(allocation.data, src, size);

    // Transition state.
    if (!(dst.state() & D3D12_RESOURCE_STATE_COPY_DEST))
        this->transition(dst, D3D12_RESOURCE_STATE_COPY_DEST);

    m_cmdList->CopyBufferRegion(dst.m_resource.Get(), dstOffset,
                                allocation.resource->m_resource.Get(), allocation.offset, size);
}

auto ink::CommandBuffer::copyTexture(const void   *src,
                                     DXGI_FORMAT   srcFormat,
                                     std::size_t   srcRowPitch,
                                     std::uint32_t width,
                                     std::uint32_t height,
                                     PixelBuffer  &dst,
                                     std::uint32_t subresource) -> void {
    // Align up row pitch.
    const std::uint32_t rowPitch  = (std::uint32_t(srcRowPitch) + 0xFF) & ~std::uint32_t(0xFF);
    const std::size_t   allocSize = (std::size_t(height) * rowPitch + 511) & ~std::size_t(511);

    DynamicBufferAllocation allocation(m_bufferAllocator.allocate(allocSize, 512U));

    { // Copy data to temporary upload buffer.
        auto       *buffer = static_cast<std::uint8_t *>(allocation.data);
        const auto *srcPtr = static_cast<const std::uint8_t *>(src);

        for (std::uint32_t i = 0; i < height; ++i) {
            std::memcpy(buffer, srcPtr, srcRowPitch);
            buffer += rowPitch;
            srcPtr += srcRowPitch;
        }
    }

    // Copy data to texture.
    D3D12_TEXTURE_COPY_LOCATION srcLoc;
    srcLoc.pResource                          = allocation.resource->m_resource.Get();
    srcLoc.Type                               = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;
    srcLoc.PlacedFootprint.Offset             = allocation.offset;
    srcLoc.PlacedFootprint.Footprint.Format   = srcFormat;
    srcLoc.PlacedFootprint.Footprint.Width    = width;
    srcLoc.PlacedFootprint.Footprint.Height   = height;
    srcLoc.PlacedFootprint.Footprint.Depth    = 1;
    srcLoc.PlacedFootprint.Footprint.RowPitch = rowPitch;

    D3D12_TEXTURE_COPY_LOCATION dstLoc;
    dstLoc.pResource        = dst.m_resource.Get();
    dstLoc.Type             = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
    dstLoc.SubresourceIndex = subresource;

    if (!(dst.state() & D3D12_RESOURCE_STATE_COPY_DEST))
        this->transition(dst, D3D12_RESOURCE_STATE_COPY_DEST);

    m_cmdList->CopyTextureRegion(&dstLoc, 0, 0, 0, &srcLoc, nullptr);
}

auto ink::CommandBuffer::setGraphicsRootSignature(RootSignature &rootSig) noexcept -> void {
    if (m_graphicsRootSignature == &rootSig)
        return;

    m_graphicsRootSignature = &rootSig;
    m_dynamicDescriptorHeap.parseGraphicsRootSignature(rootSig);
    m_cmdList->SetGraphicsRootSignature(rootSig.rootSignature());
}

auto ink::CommandBuffer::setComputeRootSignature(RootSignature &rootSig) noexcept -> void {
    if (m_computeRootSignature == &rootSig)
        return;

    m_computeRootSignature = &rootSig;
    m_dynamicDescriptorHeap.parseComputeRootSignature(rootSig);
    m_cmdList->SetComputeRootSignature(rootSig.rootSignature());
}

auto ink::CommandBuffer::setGraphicsDescriptor(std::uint32_t rootParam,
                                               std::uint32_t offset,
                                               CpuDescriptor handle) noexcept -> void {
    m_dynamicDescriptorHeap.bindGraphicsDescriptor(rootParam, offset, handle);
}

auto ink::CommandBuffer::setComputeDescriptor(std::uint32_t rootParam,
                                              std::uint32_t offset,
                                              CpuDescriptor handle) noexcept -> void {
    m_dynamicDescriptorHeap.bindComputeDescriptor(rootParam, offset, handle);
}

#include "ink/render/device.hpp"
#include "ink/core/exception.hpp"
#include "ink/core/window.hpp"
#include "ink/render/descriptor.hpp"
#include "ink/render/resource.hpp"

using namespace ink;
using Microsoft::WRL::ComPtr;

ink::RenderDevice::RenderDevice()
    : m_dxgiFactory(),
      m_dxgiAdapter(),
      m_device(),
      m_commandQueue(),
      m_fence(),
      m_fenceValue(1),
      m_allocatorPool(),
      m_freeAllocatorQueue(),
      m_freeAllocatorQueueMutex(),
      m_constantBufferViewIncrementSize(),
      m_samplerViewIncrementSize(),
      m_renderTargetViewIncrementSize(),
      m_depthStencilViewIncrementSize(),
      m_descriptorHeapPool(),
      m_freeConstantBufferViewQueue(),
      m_freeSamplerViewQueue(),
      m_freeRenderTargetViewQueue(),
      m_freeDepthStencilViewQueue(),
      m_currentConstantBufferView(),
      m_currentSamplerView(),
      m_currentRenderTargetView(),
      m_currentDepthStencilView(),
      m_freeConstantBufferViewCount(),
      m_freeSamplerViewCount(),
      m_freeRenderTargetViewCount(),
      m_freeDepthStencilViewCount(),
      m_constantBufferViewAllocateMutex(),
      m_samplerViewAllocationMutex(),
      m_renderTargetViewAllocationMutex(),
      m_depthStencilViewAllocationMutex(),
      m_freeDynViewHeapQueue(),
      m_freeDynSamplerHeapQueue(),
      m_freeDynViewHeapQueueMutex(),
      m_freeDynSamplerHeapQueueMutex(),
      m_dynBufferPagePool(),
      m_dynBufferPagePoolMutex(),
      m_freeDynBufferPageQueue(),
      m_freeDynBufferPageQueueMutex(),
      m_deletionDynBufferPageQueue(),
      m_deletionDynBufferPageQueueMutex() {
    [[maybe_unused]] HRESULT hr;

    bool debugLayerEnabled = false;
#ifndef NDEBUG
    { // Enable D3D12 debug layer.
        ComPtr<ID3D12Debug> debug;
        hr = D3D12GetDebugInterface(IID_PPV_ARGS(debug.GetAddressOf()));
        if (SUCCEEDED(hr)) {
            debug->EnableDebugLayer();
            debugLayerEnabled = true;
        }
    }
#endif

    // Create DXGI factory.
    hr = CreateDXGIFactory2(debugLayerEnabled ? DXGI_CREATE_FACTORY_DEBUG : 0,
                            IID_PPV_ARGS(m_dxgiFactory.GetAddressOf()));
    if (FAILED(hr))
        throw RenderAPIException(hr, "Failed to create DXGI factory.");

    // Enum adapters and create D3D12 device.
    for (UINT i = 0;; ++i) {
        hr = m_dxgiFactory->EnumAdapterByGpuPreference(
            i, DXGI_GPU_PREFERENCE_UNSPECIFIED,
            IID_PPV_ARGS(m_dxgiAdapter.ReleaseAndGetAddressOf()));
        if (FAILED(hr))
            throw RenderAPIException(hr, "Failed to enumerate DXGI adapters.");

        hr = D3D12CreateDevice(m_dxgiAdapter.Get(), D3D_FEATURE_LEVEL_12_0,
                               IID_PPV_ARGS(m_device.GetAddressOf()));
        if (SUCCEEDED(hr))
            break;
    }

    { // Create D3D12 command queue.
        const D3D12_COMMAND_QUEUE_DESC desc{
            /* Type     = */ D3D12_COMMAND_LIST_TYPE_DIRECT,
            /* Priority = */ D3D12_COMMAND_QUEUE_PRIORITY_NORMAL,
            /* Flags    = */ D3D12_COMMAND_QUEUE_FLAG_NONE,
            /* NodeMask = */ 0,
        };

        hr = m_device->CreateCommandQueue(&desc, IID_PPV_ARGS(m_commandQueue.GetAddressOf()));
        if (FAILED(hr))
            throw RenderAPIException(hr, "Failed to create D3D12 command queue.");
    }

    // Create fence.
    hr = m_device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(m_fence.GetAddressOf()));
    if (FAILED(hr))
        throw RenderAPIException(hr, "Failed to create D3D12 fence.");

    // Get descriptor increment size.
    m_constantBufferViewIncrementSize =
        m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
    m_samplerViewIncrementSize =
        m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER);
    m_renderTargetViewIncrementSize =
        m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
    m_depthStencilViewIncrementSize =
        m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
}

ink::RenderDevice::~RenderDevice() noexcept {
    this->sync();

    // Release all deleted dynamic buffer pages.
    while (!m_deletionDynBufferPageQueue.empty()) {
        DynamicBufferPage *page = m_deletionDynBufferPageQueue.front().second;
        delete page;
        m_deletionDynBufferPageQueue.pop();
    }

    { // Release all descriptor heaps.
        auto begin = m_descriptorHeapPool.unsafe_begin();
        auto end   = m_descriptorHeapPool.unsafe_end();
        while (begin != end) {
            (*begin)->Release();
            ++begin;
        }
    }

    { // Release all command allocators.
        auto begin = m_allocatorPool.unsafe_begin();
        auto end   = m_allocatorPool.unsafe_end();
        while (begin != end) {
            (*begin)->Release();
            ++begin;
        }
    }
}

auto ink::RenderDevice::sync() const -> void {
    const std::uint64_t value = signalFence();

    // TODO: Use thread local storage or something else to optimize event creation.
    HANDLE event = CreateEventW(nullptr, FALSE, FALSE, nullptr);
    if (event == nullptr)
        throw SystemErrorException(static_cast<std::int32_t>(GetLastError()),
                                   "Failed to create win32 event for fence synchronization.");

    m_fence->SetEventOnCompletion(value, event);
    WaitForSingleObject(event, INFINITE);
    CloseHandle(event);
}

auto ink::RenderDevice::supportRayTracing() const noexcept -> bool {
    D3D12_FEATURE_DATA_D3D12_OPTIONS5 feature{};
    HRESULT hr = m_device->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS5, &feature,
                                               static_cast<UINT>(sizeof(feature)));
    if (FAILED(hr))
        return false;

    return feature.RaytracingTier != D3D12_RAYTRACING_TIER_NOT_SUPPORTED;
}

auto ink::RenderDevice::supportUnorderedAccess(DXGI_FORMAT format) const noexcept -> bool {
    switch (format) {
    case DXGI_FORMAT_R32_FLOAT:
    case DXGI_FORMAT_R32_UINT:
    case DXGI_FORMAT_R32_SINT:
    case DXGI_FORMAT_R32G32B32A32_FLOAT:
    case DXGI_FORMAT_R32G32B32A32_UINT:
    case DXGI_FORMAT_R32G32B32A32_SINT:
    case DXGI_FORMAT_R16G16B16A16_FLOAT:
    case DXGI_FORMAT_R16G16B16A16_UINT:
    case DXGI_FORMAT_R16G16B16A16_SINT:
    case DXGI_FORMAT_R8G8B8A8_UNORM:
    case DXGI_FORMAT_R8G8B8A8_UINT:
    case DXGI_FORMAT_R8G8B8A8_SINT:
    case DXGI_FORMAT_R16_FLOAT:
    case DXGI_FORMAT_R16_UINT:
    case DXGI_FORMAT_R16_SINT:
    case DXGI_FORMAT_R8_UNORM:
    case DXGI_FORMAT_R8_UINT:
    case DXGI_FORMAT_R8_SINT:
        return true;

    case DXGI_FORMAT_R16G16B16A16_UNORM:
    case DXGI_FORMAT_R16G16B16A16_SNORM:
    case DXGI_FORMAT_R32G32_FLOAT:
    case DXGI_FORMAT_R32G32_UINT:
    case DXGI_FORMAT_R32G32_SINT:
    case DXGI_FORMAT_R10G10B10A2_UNORM:
    case DXGI_FORMAT_R10G10B10A2_UINT:
    case DXGI_FORMAT_R11G11B10_FLOAT:
    case DXGI_FORMAT_R8G8B8A8_SNORM:
    case DXGI_FORMAT_R16G16_FLOAT:
    case DXGI_FORMAT_R16G16_UNORM:
    case DXGI_FORMAT_R16G16_UINT:
    case DXGI_FORMAT_R16G16_SNORM:
    case DXGI_FORMAT_R16G16_SINT:
    case DXGI_FORMAT_R8G8_UNORM:
    case DXGI_FORMAT_R8G8_UINT:
    case DXGI_FORMAT_R8G8_SNORM:
    case DXGI_FORMAT_R8G8_SINT:
    case DXGI_FORMAT_R16_UNORM:
    case DXGI_FORMAT_R16_SNORM:
    case DXGI_FORMAT_R8_SNORM:
    case DXGI_FORMAT_A8_UNORM:
    case DXGI_FORMAT_B5G6R5_UNORM:
    case DXGI_FORMAT_B5G5R5A1_UNORM:
    case DXGI_FORMAT_B4G4R4A4_UNORM: {
        HRESULT                          hr = S_OK;
        D3D12_FEATURE_DATA_D3D12_OPTIONS feature{};

        hr = m_device->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS, &feature,
                                           static_cast<UINT>(sizeof(feature)));
        if (SUCCEEDED(hr)) {
            if (feature.TypedUAVLoadAdditionalFormats) {
                D3D12_FEATURE_DATA_FORMAT_SUPPORT formatSupport{};
                formatSupport.Format = format;

                hr = m_device->CheckFeatureSupport(D3D12_FEATURE_FORMAT_SUPPORT, &formatSupport,
                                                   static_cast<UINT>(sizeof(formatSupport)));
                if (SUCCEEDED(hr)) {
                    if ((formatSupport.Support2 & D3D12_FORMAT_SUPPORT2_UAV_TYPED_LOAD) == 0)
                        return false;

                    if ((formatSupport.Support2 & D3D12_FORMAT_SUPPORT2_UAV_TYPED_STORE) == 0)
                        return false;

                    return true;
                }
            }
        }

        return false;
    }

    default:
        return false;
    }
}

auto ink::RenderDevice::newConstantBufferView() -> ConstantBufferView {
    std::size_t ptr = acquireConstantBufferViewDescriptor();
    return {*this, D3D12_CPU_DESCRIPTOR_HANDLE{ptr}};
}

auto ink::RenderDevice::newShaderResourceView() -> ConstantBufferView {
    std::size_t ptr = acquireConstantBufferViewDescriptor();
    return {*this, D3D12_CPU_DESCRIPTOR_HANDLE{ptr}};
}

auto ink::RenderDevice::newUnorderedAccessView() -> ConstantBufferView {
    std::size_t ptr = acquireConstantBufferViewDescriptor();
    return {*this, D3D12_CPU_DESCRIPTOR_HANDLE{ptr}};
}

auto ink::RenderDevice::newRenderTargetView() -> RenderTargetView {
    std::size_t ptr = acquireRenderTargetViewDescriptor();
    return {*this, D3D12_CPU_DESCRIPTOR_HANDLE{ptr}};
}

auto ink::RenderDevice::newDepthStencilView() -> DepthStencilView {
    std::size_t ptr = acquireDepthStencilViewDescriptor();
    return {*this, D3D12_CPU_DESCRIPTOR_HANDLE{ptr}};
}

auto ink::RenderDevice::newSampler(D3D12_FILTER filter, D3D12_TEXTURE_ADDRESS_MODE addressMode)
    -> Sampler {
    std::size_t ptr = acquireSamplerDescriptor();

    const D3D12_CPU_DESCRIPTOR_HANDLE handle{ptr};
    const D3D12_SAMPLER_DESC          desc{
        /* Filter         = */ filter,
        /* AddressU       = */ addressMode,
        /* AddressV       = */ addressMode,
        /* AddressW       = */ addressMode,
        /* MipLODBias     = */ 0.0f,
        /* MaxAnisotropy  = */ 16U,
        /* ComparisonFunc = */ D3D12_COMPARISON_FUNC_LESS_EQUAL,
        /* BorderColor    = */ {0, 0, 0, 0},
        /* MinLOD         = */ 0.0f,
        /* MaxLOD         = */ D3D12_FLOAT32_MAX,
    };

    m_device->CreateSampler(&desc, handle);
    return {*this, desc, handle};
}

auto ink::RenderDevice::newSampler(const D3D12_SAMPLER_DESC &desc) -> Sampler {
    std::size_t ptr = acquireSamplerDescriptor();

    const D3D12_CPU_DESCRIPTOR_HANDLE handle{ptr};
    m_device->CreateSampler(&desc, handle);

    return {*this, desc, handle};
}

auto ink::RenderDevice::newGpuBuffer(std::size_t size) -> GpuBuffer {
    return {*this, m_device.Get(), size};
}

auto ink::RenderDevice::newStructuredBuffer(std::uint32_t elementCount, std::uint32_t elementSize)
    -> StructuredBuffer {
    return {*this, m_device.Get(), elementCount, elementSize};
}

auto ink::RenderDevice::newColorBuffer(std::uint32_t width,
                                       std::uint32_t height,
                                       DXGI_FORMAT   format,
                                       std::uint32_t sampleCount) -> ColorBuffer {
    return {*this, m_device.Get(), width, height, 1, format, 1, sampleCount};
}

auto ink::RenderDevice::newColorBuffer(std::uint32_t width,
                                       std::uint32_t height,
                                       std::uint32_t arraySize,
                                       DXGI_FORMAT   format,
                                       std::uint32_t mipLevels,
                                       std::uint32_t sampleCount) -> ColorBuffer {
    return {*this, m_device.Get(), width, height, arraySize, format, mipLevels, sampleCount};
}

auto ink::RenderDevice::newDepthBuffer(std::uint32_t width,
                                       std::uint32_t height,
                                       DXGI_FORMAT   format,
                                       std::uint32_t sampleCount) -> DepthBuffer {
    return {*this, m_device.Get(), width, height, format, sampleCount};
}

auto ink::RenderDevice::newSwapChain(Window       &window,
                                     std::uint32_t numBuffers,
                                     DXGI_FORMAT   format,
                                     bool          tearing) -> SwapChain {
    return {*this,  m_dxgiFactory.Get(), m_commandQueue.Get(), window.m_hWnd, numBuffers, format,
            tearing};
}

auto ink::RenderDevice::newSwapChain(HWND          window,
                                     std::uint32_t numBuffers,
                                     DXGI_FORMAT   format,
                                     bool          tearing) -> SwapChain {
    return {*this, m_dxgiFactory.Get(), m_commandQueue.Get(), window, numBuffers, format, tearing};
}

auto ink::RenderDevice::newCommandBuffer() -> CommandBuffer { return {*this, m_device.Get()}; }

auto ink::RenderDevice::sync(std::uint64_t fenceValue) const -> void {
    if (fenceValue <= m_fence->GetCompletedValue())
        return; // Already completed.

    // TODO: Use thread local storage or something else to optimize event creation.
    HANDLE event = CreateEventW(nullptr, FALSE, FALSE, nullptr);
    if (event == nullptr)
        throw SystemErrorException(static_cast<std::int32_t>(GetLastError()),
                                   "Failed to create win32 event for fence synchronization.");

    m_fence->SetEventOnCompletion(fenceValue, event);
    WaitForSingleObject(event, INFINITE);
    CloseHandle(event);
}

auto ink::RenderDevice::acquireCommandAllocator() -> ID3D12CommandAllocator * {
    ID3D12CommandAllocator *allocator = nullptr;

    { // Try to get one from free allocator queue.
        std::lock_guard<std::mutex> lock(m_freeAllocatorQueueMutex);
        if (!m_freeAllocatorQueue.empty()) {
            auto &front = m_freeAllocatorQueue.front();
            if (front.first <= m_fence->GetCompletedValue()) {
                allocator = front.second;
                m_freeAllocatorQueue.pop();
            }
        }
    }

    if (allocator != nullptr) {
        allocator->Reset();
        return allocator;
    }

    // No available free allocator, create a new one.
    HRESULT hr =
        m_device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&allocator));
    if (FAILED(hr))
        throw RenderAPIException(hr, "Failed to create direct command list.");

    m_allocatorPool.push(allocator);
    return allocator;
}

auto ink::RenderDevice::releaseCommandAllocator(std::uint64_t           fenceValue,
                                                ID3D12CommandAllocator *allocator) noexcept
    -> void {
    std::lock_guard<std::mutex> lock(m_freeAllocatorQueueMutex);
    m_freeAllocatorQueue.emplace(fenceValue, allocator);
}

auto ink::RenderDevice::acquireConstantBufferViewDescriptor() -> std::size_t {
    { // Try to get one from free descriptor queue.
        std::size_t handle;
        if (m_freeConstantBufferViewQueue.try_pop(handle))
            return handle;
    }

    std::lock_guard<std::mutex> lock(m_constantBufferViewAllocateMutex);

    // No more free descriptors, create a new descriptor heap.
    if (m_freeConstantBufferViewCount == 0) {
        ID3D12DescriptorHeap *newHeap;

        const D3D12_DESCRIPTOR_HEAP_DESC desc{
            /* Type           = */ D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV,
            /* NumDescriptors = */ 256U,
            /* Flags          = */ D3D12_DESCRIPTOR_HEAP_FLAG_NONE,
            /* NodeMask       = */ 0,
        };

        HRESULT hr = m_device->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&newHeap));
        if (FAILED(hr))
            throw RenderAPIException(hr,
                                     "Failed to create new constant buffer view descriptor heap.");

        m_currentConstantBufferView   = newHeap->GetCPUDescriptorHandleForHeapStart().ptr;
        m_freeConstantBufferViewCount = desc.NumDescriptors;

        m_descriptorHeapPool.push(newHeap);
    }

    std::size_t result = m_currentConstantBufferView;

    m_currentConstantBufferView += m_constantBufferViewIncrementSize;
    m_freeConstantBufferViewCount -= 1;

    return result;
}

auto ink::RenderDevice::releaseConstantBufferViewDescriptor(std::size_t ptr) noexcept -> void {
    m_freeConstantBufferViewQueue.push(ptr);
}

auto ink::RenderDevice::acquireSamplerDescriptor() -> std::size_t {
    { // Try to get one from free descriptor queue.
        std::size_t handle;
        if (m_freeSamplerViewQueue.try_pop(handle))
            return handle;
    }

    std::lock_guard<std::mutex> lock(m_samplerViewAllocationMutex);

    // No more free descriptors, create a new descriptor heap.
    if (m_freeSamplerViewCount == 0) {
        ID3D12DescriptorHeap *newHeap;

        const D3D12_DESCRIPTOR_HEAP_DESC desc{
            /* Type           = */ D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER,
            /* NumDescriptors = */ 32U,
            /* Flags          = */ D3D12_DESCRIPTOR_HEAP_FLAG_NONE,
            /* NodeMask       = */ 0,
        };

        HRESULT hr = m_device->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&newHeap));
        if (FAILED(hr))
            throw RenderAPIException(hr, "Failed to create new sampler descriptor heap.");

        m_currentSamplerView   = newHeap->GetCPUDescriptorHandleForHeapStart().ptr;
        m_freeSamplerViewCount = desc.NumDescriptors;

        m_descriptorHeapPool.push(newHeap);
    }

    std::size_t result = m_currentSamplerView;

    m_currentSamplerView += m_samplerViewIncrementSize;
    m_freeSamplerViewCount -= 1;

    return result;
}

auto ink::RenderDevice::releaseSamplerDescriptor(std::size_t ptr) noexcept -> void {
    m_freeSamplerViewQueue.push(ptr);
}

auto ink::RenderDevice::acquireRenderTargetViewDescriptor() -> std::size_t {
    { // Try to get one from free descriptor queue.
        std::size_t handle;
        if (m_freeRenderTargetViewQueue.try_pop(handle))
            return handle;
    }

    std::lock_guard<std::mutex> lock(m_renderTargetViewAllocationMutex);

    // No more free descriptors, create a new descriptor heap.
    if (m_freeRenderTargetViewCount == 0) {
        ID3D12DescriptorHeap *newHeap;

        const D3D12_DESCRIPTOR_HEAP_DESC desc{
            /* Type           = */ D3D12_DESCRIPTOR_HEAP_TYPE_RTV,
            /* NumDescriptors = */ 64U,
            /* Flags          = */ D3D12_DESCRIPTOR_HEAP_FLAG_NONE,
            /* NodeMask       = */ 0,
        };

        HRESULT hr = m_device->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&newHeap));
        if (FAILED(hr))
            throw RenderAPIException(hr,
                                     "Failed to create new render target view descriptor heap.");

        m_currentRenderTargetView   = newHeap->GetCPUDescriptorHandleForHeapStart().ptr;
        m_freeRenderTargetViewCount = desc.NumDescriptors;

        m_descriptorHeapPool.push(newHeap);
    }

    std::size_t result = m_currentRenderTargetView;

    m_currentRenderTargetView += m_renderTargetViewIncrementSize;
    m_freeRenderTargetViewCount -= 1;

    return result;
}

auto ink::RenderDevice::releaseRenderTargetViewDescriptor(std::size_t ptr) noexcept -> void {
    m_freeRenderTargetViewQueue.push(ptr);
}

auto ink::RenderDevice::acquireDepthStencilViewDescriptor() -> std::size_t {
    { // Try to get one from free descriptor queue.
        std::size_t handle;
        if (m_freeDepthStencilViewQueue.try_pop(handle))
            return handle;
    }

    std::lock_guard<std::mutex> lock(m_depthStencilViewAllocationMutex);

    // No more free descriptors, create a new descriptor heap.
    if (m_freeDepthStencilViewCount == 0) {
        ID3D12DescriptorHeap *newHeap;

        const D3D12_DESCRIPTOR_HEAP_DESC desc{
            /* Type           = */ D3D12_DESCRIPTOR_HEAP_TYPE_DSV,
            /* NumDescriptors = */ 64U,
            /* Flags          = */ D3D12_DESCRIPTOR_HEAP_FLAG_NONE,
            /* NodeMask       = */ 0,
        };

        HRESULT hr = m_device->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&newHeap));
        if (FAILED(hr))
            throw RenderAPIException(hr,
                                     "Failed to create new depth stencil view descriptor heap.");

        m_currentDepthStencilView   = newHeap->GetCPUDescriptorHandleForHeapStart().ptr;
        m_freeDepthStencilViewCount = desc.NumDescriptors;

        m_descriptorHeapPool.push(newHeap);
    }

    std::size_t result = m_currentDepthStencilView;

    m_currentDepthStencilView += m_depthStencilViewIncrementSize;
    m_freeDepthStencilViewCount -= 1;

    return result;
}

auto ink::RenderDevice::releaseDepthStencilViewDescriptor(std::size_t ptr) noexcept -> void {
    m_freeDepthStencilViewQueue.push(ptr);
}

auto ink::RenderDevice::acquireDynamicViewHeap() -> ID3D12DescriptorHeap * {
    { // Try to get one from retired dynamic CBV/SRV/UAV heap queue.
        std::lock_guard<std::mutex> lock(m_freeDynViewHeapQueueMutex);
        if (!m_freeDynViewHeapQueue.empty()) {
            auto &front = m_freeDynViewHeapQueue.front();
            if (front.first <= m_fence->GetCompletedValue()) {
                ID3D12DescriptorHeap *heap = front.second;
                m_freeDynViewHeapQueue.pop();
                return heap;
            }
        }
    }

    // Create a new shader-visible descriptor heap.
    ID3D12DescriptorHeap *heap;

    const D3D12_DESCRIPTOR_HEAP_DESC desc{
        /* Type           = */ D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV,
        /* NumDescriptors = */ 1024U,
        /* Flags          = */ D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE,
        /* NodeMask       = */ 0,
    };

    HRESULT hr = m_device->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&heap));
    if (FAILED(hr))
        throw RenderAPIException(hr, "Failed to create new dynamic view descriptor heap.");

    m_descriptorHeapPool.push(heap);
    return heap;
}

auto ink::RenderDevice::releaseDynamicViewHeaps(std::uint64_t          fenceValue,
                                                std::size_t            count,
                                                ID3D12DescriptorHeap **heaps) noexcept -> void {
    ID3D12DescriptorHeap      **heapEnd = heaps + count;
    std::lock_guard<std::mutex> lock(m_freeDynViewHeapQueueMutex);
    while (heaps != heapEnd) {
        m_freeDynViewHeapQueue.emplace(fenceValue, *heaps);
        ++heaps;
    }
}

auto ink::RenderDevice::acquireDynamicSamplerHeap() -> ID3D12DescriptorHeap * {
    { // Try to get one from retired dynamic sampler heap queue.
        std::lock_guard<std::mutex> lock(m_freeDynSamplerHeapQueueMutex);
        if (!m_freeDynSamplerHeapQueue.empty()) {
            auto &front = m_freeDynSamplerHeapQueue.front();
            if (front.first <= m_fence->GetCompletedValue()) {
                ID3D12DescriptorHeap *heap = front.second;
                m_freeDynSamplerHeapQueue.pop();
                return heap;
            }
        }
    }

    // Create a new shader-visible descriptor heap.
    ID3D12DescriptorHeap *heap;

    const D3D12_DESCRIPTOR_HEAP_DESC desc{
        /* Type           = */ D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER,
        /* NumDescriptors = */ 256U,
        /* Flags          = */ D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE,
        /* NodeMask       = */ 0,
    };

    HRESULT hr = m_device->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&heap));
    if (FAILED(hr))
        throw RenderAPIException(hr, "Failed to create new dynamic sampler descriptor heap.");

    m_descriptorHeapPool.push(heap);
    return heap;
}

auto ink::RenderDevice::releaseDynamicSamplerHeaps(std::uint64_t          fenceValue,
                                                   std::size_t            count,
                                                   ID3D12DescriptorHeap **heaps) noexcept -> void {
    ID3D12DescriptorHeap      **heapEnd = heaps + count;
    std::lock_guard<std::mutex> lock(m_freeDynSamplerHeapQueueMutex);
    while (heaps != heapEnd) {
        m_freeDynSamplerHeapQueue.emplace(fenceValue, *heaps);
        ++heaps;
    }
}

namespace {

/// @brief
///   Default dynamic buffer page size is 16Mib.
constexpr const std::size_t DYNAMIC_BUFFER_PAGE_SIZE = 0x1000000;

} // namespace

auto ink::RenderDevice::acquireDynamicBufferPage(std::size_t size) -> DynamicBufferPage * {
    // Align up size.
    size = ((size + 0xFF) & ~std::size_t(0xFF));

    if (size <= DYNAMIC_BUFFER_PAGE_SIZE) {
        { // Try to get a free page from free page queue.
            std::lock_guard<std::mutex> lock(m_freeDynBufferPageQueueMutex);
            if (!m_freeDynBufferPageQueue.empty()) {
                auto &front = m_freeDynBufferPageQueue.front();
                if (front.first <= m_fence->GetCompletedValue()) {
                    DynamicBufferPage *page = front.second;
                    m_freeDynBufferPageQueue.pop();
                    return page;
                }
            }
        }

        { // Create a new page and return it.
            DynamicBufferPage           newPage(m_device.Get(), DYNAMIC_BUFFER_PAGE_SIZE);
            std::lock_guard<std::mutex> lock(m_dynBufferPagePoolMutex);
            return std::addressof(m_dynBufferPagePool.emplace(std::move(newPage)));
        }
    }

    return new DynamicBufferPage(m_device.Get(), size);
}

auto ink::RenderDevice::releaseDynamicBufferPages(std::uint64_t       fenceValue,
                                                  std::size_t         count,
                                                  DynamicBufferPage **pages) noexcept -> void {
    DynamicBufferPage **pageEnd = pages + count;

    { // Free default pages.
        std::lock_guard<std::mutex> lock(m_freeDynBufferPageQueueMutex);
        for (auto *page = pages; page != pageEnd; ++page) {
            if ((*page)->size() <= DYNAMIC_BUFFER_PAGE_SIZE)
                m_freeDynBufferPageQueue.emplace(fenceValue, *page);
        }
    }

    { // Free deletion pages.
        const std::uint64_t         currentFence = m_fence->GetCompletedValue();
        std::lock_guard<std::mutex> lock(m_deletionDynBufferPageQueueMutex);
        for (auto *page = pages; page != pageEnd; ++page) {
            if ((*page)->size() > DYNAMIC_BUFFER_PAGE_SIZE)
                m_deletionDynBufferPageQueue.emplace(fenceValue, *page);
        }

        while (!m_deletionDynBufferPageQueue.empty()) {
            auto &front = m_deletionDynBufferPageQueue.front();
            if (front.first <= currentFence) {
                delete front.second;
                m_deletionDynBufferPageQueue.pop();
            } else {
                break;
            }
        }
    }
}

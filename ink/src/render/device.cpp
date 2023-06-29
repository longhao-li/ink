#include "ink/render/device.h"
#include "ink/core/assert.h"

using namespace ink;
using Microsoft::WRL::ComPtr;

namespace {

/// @brief
///   D3D12 debug layer message callback function.
///
/// @param category
///   D3D12 debug message category.
/// @param severity
///   Message severity level.
/// @param ID
///   Message ID.
/// @param description
///   The full message content.
/// @param context
///   User-defined object pointer. Not used here.
[[maybe_unused]]
auto CALLBACK messageCallback([[maybe_unused]] D3D12_MESSAGE_CATEGORY category,
                              D3D12_MESSAGE_SEVERITY                  severity,
                              [[maybe_unused]] D3D12_MESSAGE_ID       ID,
                              LPCSTR                                  description,
                              [[maybe_unused]] void                  *context) noexcept -> void {
    std::string_view   message(description);
    FormatMemoryBuffer buffer;

    // We assume that descripton is always written in English.
    for (const auto c : message)
        buffer.push_back(static_cast<char16_t>(c));

    String msg(buffer.data(), buffer.size());
    switch (severity) {
    case D3D12_MESSAGE_SEVERITY_MESSAGE:
        logTrace(msg);
        break;

    case D3D12_MESSAGE_SEVERITY_INFO:
        logInfo(msg);
        break;

    case D3D12_MESSAGE_SEVERITY_WARNING:
        logWarning(msg);
        break;

    case D3D12_MESSAGE_SEVERITY_ERROR:
        logError(msg);
        break;

    case D3D12_MESSAGE_SEVERITY_CORRUPTION:
        logFatal(msg);
        break;

    default:
        break;
    }
}

} // namespace

ink::RenderDevice::RenderDevice() noexcept
    : m_factory(),
      m_adapter(),
      m_device(),
      m_infoQueue(),
      m_cmdQueue(),
      m_fence(),
      m_nextFenceValue(1),
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
      m_depthStencilViewAllocationMutex() {
    [[maybe_unused]] HRESULT hr;

    bool debugLayerEnabled = false;
#ifndef NDEBUG
    { // Enable D3D12 debug layer.
        ComPtr<ID3D12Debug> debug;
        hr = D3D12GetDebugInterface(IID_PPV_ARGS(debug.GetAddressOf()));
        if (SUCCEEDED(hr)) {
            debug->EnableDebugLayer();
            debugLayerEnabled = true;
            logTrace(u"D3D12 debug layer enabled.");
        } else {
            logWarning(u"Debug build is enabled, but failed to enable D3D12 debug layer: 0x{:X}.",
                       static_cast<std::uint32_t>(hr));
        }
    }
#endif

    // Create DXGI factory.
    hr = CreateDXGIFactory2(debugLayerEnabled ? DXGI_CREATE_FACTORY_DEBUG : 0,
                            IID_PPV_ARGS(m_factory.GetAddressOf()));
    inkAssert(SUCCEEDED(hr), u"Failed to create DXGI factory: 0x{:X}.",
              static_cast<std::uint32_t>(hr));

    // Enum adapters and create D3D12 device.
    for (UINT i = 0;; ++i) {
        hr =
            m_factory->EnumAdapterByGpuPreference(i, DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE,
                                                  IID_PPV_ARGS(m_adapter.ReleaseAndGetAddressOf()));
        inkAssert(SUCCEEDED(hr), u"Failed to enumerate DXGI adapters: 0x{:X}.",
                  static_cast<std::uint32_t>(hr));

        hr = D3D12CreateDevice(m_adapter.Get(), D3D_FEATURE_LEVEL_12_0,
                               IID_PPV_ARGS(m_device.GetAddressOf()));
        if (SUCCEEDED(hr))
            break;
    }

#ifndef NDEBUG
    // Enable info queue messenger if debug layer is enabled.
    if (debugLayerEnabled) {
        hr = m_device.As(&m_infoQueue);
        if (FAILED(hr)) {
            logWarning(
                u"D3D12 debug layer is enabled, but failed to retrieve ID3D12InfoQueue1: 0x{:X}. "
                u"ID3D12InfoQueue1 is only supported by Windows 11 and later versions.",
                static_cast<std::uint32_t>(hr));
        } else {
            DWORD cookie = 0;

            hr = m_infoQueue->RegisterMessageCallback(
                messageCallback, D3D12_MESSAGE_CALLBACK_FLAG_NONE, nullptr, &cookie);
            if (FAILED(hr))
                logError(
                    u"Failed to register message callback function to ID3D12InfoQueue1: 0x{:X}.",
                    static_cast<std::uint32_t>(hr));
        }
    }
#endif

    { // Create D3D12 command queue.
        const D3D12_COMMAND_QUEUE_DESC desc{
            /* Type     = */ D3D12_COMMAND_LIST_TYPE_DIRECT,
            /* Priority = */ D3D12_COMMAND_QUEUE_PRIORITY_NORMAL,
            /* Flags    = */ D3D12_COMMAND_QUEUE_FLAG_NONE,
            /* NodeMask = */ 0,
        };

        hr = m_device->CreateCommandQueue(&desc, IID_PPV_ARGS(m_cmdQueue.GetAddressOf()));
        inkAssert(SUCCEEDED(hr), u"Failed to create ID3D12CommandQueue: 0x{:X}.",
                  static_cast<std::uint32_t>(hr));
    }

    // Create fence.
    hr = m_device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(m_fence.GetAddressOf()));
    inkAssert(SUCCEEDED(hr), u"Failed to create ID3D12Fence1: 0x{:X}.",
              static_cast<std::uint32_t>(hr));

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

    { // Release all CPU descriptor heaps.
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

auto ink::RenderDevice::sync(std::uint64_t fenceValue) const noexcept -> void {
    if (isFenceReached(fenceValue))
        return;

    struct FenceEvent {
        HANDLE handle;

        FenceEvent() noexcept : handle(CreateEventW(nullptr, FALSE, FALSE, nullptr)) {}
        ~FenceEvent() noexcept {
            CloseHandle(handle);
        }
    };

    static thread_local FenceEvent fenceEvent;

    const HANDLE eventHandle = fenceEvent.handle;
    m_fence->SetEventOnCompletion(fenceValue, eventHandle);
    WaitForSingleObject(eventHandle, INFINITE);
}

auto ink::RenderDevice::newCommandAllocator() noexcept -> ID3D12CommandAllocator * {
    ID3D12CommandAllocator *allocator = nullptr;

    { // Try to get one from free allocator queue.
        std::lock_guard<std::mutex> lock(m_freeAllocatorQueueMutex);
        if (!m_freeAllocatorQueue.empty()) {
            auto &front = m_freeAllocatorQueue.front();
            if (isFenceReached(front.first)) {
                allocator = front.second;
                m_freeAllocatorQueue.pop();
            }
        }
    }

    if (allocator != nullptr) {
        allocator->Reset();
        return allocator;
    }

    // Try to create a new command allocator.
    [[maybe_unused]] HRESULT hr;
    hr = m_device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&allocator));
    inkAssert(SUCCEEDED(hr), u"Failed to create ID3D12CommandAllocator: 0x{:X}.",
              static_cast<std::uint32_t>(hr));

    m_allocatorPool.push(allocator);
    return allocator;
}

auto ink::RenderDevice::freeCommandAllocator(std::uint64_t           fenceValue,
                                             ID3D12CommandAllocator *allocator) noexcept -> void {
    std::lock_guard<std::mutex> lock(m_freeAllocatorQueueMutex);
    m_freeAllocatorQueue.emplace(fenceValue, allocator);
}

auto ink::RenderDevice::supportRayTracing() const noexcept -> bool {
    HRESULT                           hr;
    D3D12_FEATURE_DATA_D3D12_OPTIONS5 feature{};

    hr = m_device->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS5, &feature, sizeof(feature));
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

auto ink::RenderDevice::newConstantBufferView() noexcept -> D3D12_CPU_DESCRIPTOR_HANDLE {
    [[maybe_unused]] HRESULT hr;

    { // Try to get one from free handle queue.
        D3D12_CPU_DESCRIPTOR_HANDLE handle;
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

        hr = m_device->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&newHeap));
        inkAssert(SUCCEEDED(hr), u"Failed to create new constant buffer view: 0x{:X}.",
                  static_cast<std::uint32_t>(hr));

        m_currentConstantBufferView   = newHeap->GetCPUDescriptorHandleForHeapStart();
        m_freeConstantBufferViewCount = desc.NumDescriptors;

        m_descriptorHeapPool.push(newHeap);
    }

    D3D12_CPU_DESCRIPTOR_HANDLE result = m_currentConstantBufferView;

    m_currentConstantBufferView.ptr += m_constantBufferViewIncrementSize;
    m_freeConstantBufferViewCount -= 1;

    return result;
}

auto ink::RenderDevice::freeConstantBufferView(D3D12_CPU_DESCRIPTOR_HANDLE handle) noexcept
    -> void {
    m_freeConstantBufferViewQueue.push(handle);
}

auto ink::RenderDevice::newSamplerView() noexcept -> D3D12_CPU_DESCRIPTOR_HANDLE {
    [[maybe_unused]] HRESULT hr;

    { // Try to get one from free handle queue.
        D3D12_CPU_DESCRIPTOR_HANDLE handle;
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

        hr = m_device->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&newHeap));
        inkAssert(SUCCEEDED(hr), u"Failed to create sampler CPU descriptor heap: 0x{:X}.",
                  static_cast<std::uint32_t>(hr));

        m_currentSamplerView   = newHeap->GetCPUDescriptorHandleForHeapStart();
        m_freeSamplerViewCount = desc.NumDescriptors;

        m_descriptorHeapPool.push(newHeap);
    }

    D3D12_CPU_DESCRIPTOR_HANDLE result = m_currentSamplerView;

    m_currentSamplerView.ptr += m_samplerViewIncrementSize;
    m_freeSamplerViewCount -= 1;

    return result;
}

auto ink::RenderDevice::freeSamplerView(D3D12_CPU_DESCRIPTOR_HANDLE handle) noexcept -> void {
    m_freeSamplerViewQueue.push(handle);
}

auto ink::RenderDevice::newRenderTargetView() noexcept -> D3D12_CPU_DESCRIPTOR_HANDLE {
    [[maybe_unused]] HRESULT hr;

    { // Try to get one from free handle queue.
        D3D12_CPU_DESCRIPTOR_HANDLE handle;
        if (m_freeRenderTargetViewQueue.try_pop(handle))
            return handle;
    }

    std::lock_guard<std::mutex> lock(m_renderTargetViewAllocationMutex);

    // No more descriptors, create a new descriptor heap.
    if (m_freeRenderTargetViewCount == 0) {
        ID3D12DescriptorHeap *newHeap;

        const D3D12_DESCRIPTOR_HEAP_DESC desc{
            /* Type           = */ D3D12_DESCRIPTOR_HEAP_TYPE_RTV,
            /* NumDescriptors = */ 64U,
            /* Flags          = */ D3D12_DESCRIPTOR_HEAP_FLAG_NONE,
            /* NodeMask       = */ 0,
        };

        hr = m_device->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&newHeap));
        inkAssert(SUCCEEDED(hr),
                  u"Failed to create render target view CPU descriptor heap: 0x{:X}.",
                  static_cast<std::uint32_t>(hr));

        m_currentRenderTargetView   = newHeap->GetCPUDescriptorHandleForHeapStart();
        m_freeRenderTargetViewCount = desc.NumDescriptors;

        m_descriptorHeapPool.push(newHeap);
    }

    D3D12_CPU_DESCRIPTOR_HANDLE result = m_currentRenderTargetView;

    m_currentRenderTargetView.ptr += m_renderTargetViewIncrementSize;
    m_freeRenderTargetViewCount -= 1;

    return result;
}

auto ink::RenderDevice::freeRenderTargetView(D3D12_CPU_DESCRIPTOR_HANDLE handle) noexcept -> void {
    m_freeRenderTargetViewQueue.push(handle);
}

auto ink::RenderDevice::newDepthStencilView() noexcept -> D3D12_CPU_DESCRIPTOR_HANDLE {
    [[maybe_unused]] HRESULT hr;

    { // Try to get one from free handle queue.
        D3D12_CPU_DESCRIPTOR_HANDLE handle;
        if (m_freeDepthStencilViewQueue.try_pop(handle))
            return handle;
    }

    std::lock_guard<std::mutex> lock(m_depthStencilViewAllocationMutex);

    // No more descriptors, create a new descriptor heap.
    if (m_freeDepthStencilViewCount == 0) {
        ID3D12DescriptorHeap *newHeap;

        const D3D12_DESCRIPTOR_HEAP_DESC desc{
            /* Type           = */ D3D12_DESCRIPTOR_HEAP_TYPE_DSV,
            /* NumDescriptors = */ 64U,
            /* Flags          = */ D3D12_DESCRIPTOR_HEAP_FLAG_NONE,
            /* NodeMask       = */ 0,
        };

        hr = m_device->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&newHeap));
        inkAssert(SUCCEEDED(hr),
                  u"Failed to create new depth stencil view CPU descriptor heap: 0x{:X}.",
                  static_cast<std::uint32_t>(hr));

        m_currentDepthStencilView   = newHeap->GetCPUDescriptorHandleForHeapStart();
        m_freeDepthStencilViewCount = desc.NumDescriptors;

        m_descriptorHeapPool.push(newHeap);
    }

    D3D12_CPU_DESCRIPTOR_HANDLE result = m_currentDepthStencilView;

    m_currentDepthStencilView.ptr += m_depthStencilViewIncrementSize;
    m_freeDepthStencilViewCount -= 1;

    return result;
}

auto ink::RenderDevice::freeDepthStencilView(D3D12_CPU_DESCRIPTOR_HANDLE handle) noexcept -> void {
    m_freeDepthStencilViewQueue.push(handle);
}

auto ink::RenderDevice::singleton() noexcept -> RenderDevice & {
    static RenderDevice instance;
    return instance;
}

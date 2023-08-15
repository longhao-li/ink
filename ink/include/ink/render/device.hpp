#pragma once

#include "command_buffer.hpp"

#include <concurrent_queue.h>
#include <d3d12.h>
#include <dxgi1_6.h>
#include <wrl/client.h>

#include <atomic>
#include <mutex>
#include <queue>
#include <stack>

namespace ink {

class ConstantBufferView;
class RenderTargetView;
class DepthStencilView;
class Sampler;
class GpuBuffer;
class StructuredBuffer;
class ColorBuffer;
class DepthBuffer;
class Texture2D;
class Window;
class SwapChain;

class RenderDevice {
public:
    /// @brief
    ///   Create and initialize a render device.
    ///
    /// @throw RenderAPIException
    ///   Thrown if failed to initialize D3D12.
    InkExport RenderDevice();

    /// @brief
    ///   Copy constructor of render device is disabled.
    RenderDevice(const RenderDevice &) = delete;

    /// @brief
    ///   Move constructor of render device is disabled.
    RenderDevice(RenderDevice &&) = delete;

    /// @brief
    ///   Release all D3D12 resources and destroy this render device.
    InkExport ~RenderDevice() noexcept;

    /// @brief
    ///   Copy assignment of render device is disabled.
    auto operator=(const RenderDevice &) = delete;

    /// @brief
    ///   Move assignment of render device is disabled.
    auto operator=(RenderDevice &&) = delete;

    /// @brief
    ///   Block current thread until all tasks submitted to GPU before this method call are
    ///   completed.
    ///
    /// @throw SystemErrorException
    ///   Thrown if failed to create Win32 event object.
    InkExport auto sync() const -> void;

    /// @brief
    ///   Checks if this render device supports DirectX ray tracing.
    ///
    /// @return
    ///   A boolean value that indicates whether this render device supports DXR.
    /// @retval true
    ///   This render device supports DXR.
    /// @retval false
    ///   This render device does not support DXR.
    [[nodiscard]] InkExport auto supportRayTracing() const noexcept -> bool;

    /// @brief
    ///   Checks if the specified pixel format supports unordered access.
    ///
    /// @param format
    ///   The pixel format to be checked.
    ///
    /// @return
    ///   A boolean value that indicates whether the specified pixel format supports unrodered
    ///   access.
    /// @retval true
    ///   The specified pixel format supports unordered access.
    /// @retval false
    ///   The specified pixel format does not support unordered access.
    [[nodiscard]] InkExport auto supportUnorderedAccess(DXGI_FORMAT format) const noexcept -> bool;

    /// @brief
    ///   Create a new constant buffer view. Constant buffer view could also be used as shader
    ///   resource view and unordered access view.
    ///
    /// @return
    ///   A new constant buffer view.
    ///
    /// @throw RenderAPIException
    ///   Thrown if failed to create new CBV/SRV/UAV descriptor heap.
    [[nodiscard]] InkExport auto newConstantBufferView() -> ConstantBufferView;

    /// @brief
    ///   Create a new shader resource view. Shader resource view could also be used as constant
    ///   buffer view and unordered access view.
    ///
    /// @return
    ///   A new shader resource view.
    ///
    /// @throw RenderAPIException
    ///   Thrown if failed to create new CBV/SRV/UAV descriptor heap.
    [[nodiscard]] InkExport auto newShaderResourceView() -> ConstantBufferView;

    /// @brief
    ///   Create a new unordered access view. Unordered access view could also be used as constant
    ///   buffer view and shader resource view.
    ///
    /// @return
    ///   A new unordered access view.
    ///
    /// @throw RenderAPIException
    ///   Thrown if failed to create new CBV/SRV/UAV descriptor heap.
    [[nodiscard]] InkExport auto newUnorderedAccessView() -> ConstantBufferView;

    /// @brief
    ///   Create a new render target view.
    ///
    /// @return
    ///   A new render target view.
    ///
    /// @throw RenderAPIException
    ///   Thrown if failed to create new render target view descriptor heap.
    [[nodiscard]] InkExport auto newRenderTargetView() -> RenderTargetView;

    /// @brief
    ///   Create a new depth stencil view.
    ///
    /// @return
    ///   A new depth stencil view.
    ///
    /// @throw RenderAPIException
    ///   Thrown if failed to create new depth stencil view descriptor heap.
    [[nodiscard]] InkExport auto newDepthStencilView() -> DepthStencilView;

    /// @brief
    ///   Create a new sampler. Most parameters are set to default values. The default values are as
    ///   follows:
    ///
    ///   - MipLODBias: 0.0f
    ///   - MaxAnisotropy: 16
    ///   - ComparisonFunc: D3D12_COMPARISON_FUNC_LESS_EQUAL
    ///   - BorderColor: {0, 0, 0, 0}
    ///   - MinLOD: 0.0f
    ///   - MaxLOD: D3D12_FLOAT32_MAX
    ///
    /// @param filter
    ///   Filter type of this sampler.
    /// @param addressMode
    ///   Address mode of this sampler. AddressU, AddressV and AddressW are set to the same value.
    ///
    /// @return
    ///   A new sampler.
    ///
    /// @throw RenderAPIException
    ///   Thrown if failed to create new sampler descriptor heap.
    [[nodiscard]] InkExport auto newSampler(D3D12_FILTER               filter,
                                            D3D12_TEXTURE_ADDRESS_MODE addressMode) -> Sampler;

    /// @brief
    ///   Create a new sampler.
    ///
    /// @param desc
    ///   D3D12 sampler desc that describes how to create this sampler.
    ///
    /// @return
    ///   A new sampler.
    ///
    /// @throw RenderAPIException
    ///   Thrown if failed to create new sampler descriptor heap.
    [[nodiscard]] InkExport auto newSampler(const D3D12_SAMPLER_DESC &desc) -> Sampler;

    /// @brief
    ///   Create a new GPU buffer.
    ///
    /// @param size
    ///   Expected size in byte of this GPU buffer. The actual size may be greater due to alignment.
    ///
    /// @return
    ///   The new GPU buffer.
    ///
    /// @throw RenderAPIException
    ///   Thrown if failed to create the GPU buffer.
    [[nodiscard]] InkExport auto newGpuBuffer(std::size_t size) -> GpuBuffer;

    /// @brief
    ///   Create a new structured buffer.
    ///
    /// @param elementCount
    ///   Number of elements in this structured buffer.
    /// @param elementSize
    ///   Size in byte of each element in this structured buffer.
    ///
    /// @return
    ///   The new structured buffer.
    ///
    /// @throw RenderAPIException
    ///   Thrown if failed to create the structured buffer.
    [[nodiscard]] InkExport auto newStructuredBuffer(std::uint32_t elementCount,
                                                     std::uint32_t elementSize) -> StructuredBuffer;

    /// @brief
    ///   Create a new 2D color buffer. Color buffer could be used as render target.
    ///
    /// @param width
    ///   Width in pixel of the color buffer.
    /// @param height
    ///   Height in pixel of the color buffer.
    /// @param format
    ///   Pixel format of the color buffer.
    /// @param sampleCount
    ///   Sample count of the color buffer. This value will be set to 1 if 0 is passed. Enabling
    ///   multi-sample will disable unordered access for the color buffer.
    [[nodiscard]] InkExport auto newColorBuffer(std::uint32_t width,
                                                std::uint32_t height,
                                                DXGI_FORMAT   format,
                                                std::uint32_t sampleCount = 1) -> ColorBuffer;

    /// @brief
    ///   Create a new color buffer. Color buffer could be used as render target.
    ///
    /// @param width
    ///   Width in pixel of the color buffer.
    /// @param height
    ///   Height in pixel of the color buffer.
    /// @param arraySize
    ///   Number of 2D textures in the color buffer. Use 1 to create a 2D color buffer instead of 2D
    ///   texture array.
    /// @param format
    ///   Pixel format of the color buffer.
    /// @param mipLevels
    ///   Maximum supported mipmap level by this color buffer. This value will always be clamped
    ///   between 1 and the maximum available value. Pass 0 to use the maximum available level.
    ///
    /// @return
    ///   The new color buffer.
    ///
    /// @throw RenderAPIException
    ///   Thrown if failed to create the color buffer.
    [[nodiscard]] InkExport auto newColorBuffer(std::uint32_t width,
                                                std::uint32_t height,
                                                std::uint32_t arraySize,
                                                DXGI_FORMAT   format,
                                                std::uint32_t mipLevels   = 1,
                                                std::uint32_t sampleCount = 1) -> ColorBuffer;

    /// @brief
    ///   Create a new depth buffer. Depth buffer may also be used as stencil buffer which depends
    ///   on the pixel format.
    ///
    /// @param width
    ///   Width in pixel of the depth buffer.
    /// @param height
    ///   Height in pixel of the depth buffer.
    /// @param format
    ///   Pixel format of the depth buffer.
    /// @param sampleCount
    ///   Sample count of the depth buffer. This value will be set to 1 if 0 is passed. Enabling
    ///   multi-sample will disable unordered access for the depth buffer.
    [[nodiscard]] InkExport auto newDepthBuffer(std::uint32_t width,
                                                std::uint32_t height,
                                                DXGI_FORMAT   format,
                                                std::uint32_t sampleCount = 1) -> DepthBuffer;

    /// @brief
    ///   Create a new swap chain for the specified window.
    ///
    /// @param window
    ///   The window that this swap chain is created for.
    /// @param numBuffers
    ///   Expected number of back buffers in this swap chain. This value will always be clamped
    ///   between 2 an 3.
    /// @param format
    ///   Back buffer pixel format.
    /// @param tearing
    ///   Specifies whether to enable variable refresh rate for this swap chain. Tearing will not be
    ///   enabled if not supported and no exception will be thrown.
    ///
    /// @throw RenderAPIException
    ///   Thrown if failed to create the swap chain and get back buffers.
    [[nodiscard]] InkExport auto newSwapChain(Window       &window,
                                              std::uint32_t numBuffers = 2,
                                              DXGI_FORMAT   format     = DXGI_FORMAT_R8G8B8A8_UNORM,
                                              bool          tearing    = true) -> SwapChain;

    /// @brief
    ///   Create a new swap chain for the specified window.
    ///
    /// @param window
    ///   The window that this swap chain is created for.
    /// @param numBuffers
    ///   Expected number of back buffers in this swap chain. This value will always be clamped
    ///   between 2 an 3.
    /// @param format
    ///   Back buffer pixel format.
    /// @param tearing
    ///   Specifies whether to enable variable refresh rate for this swap chain. Tearing will not be
    ///   enabled if not supported and no exception will be thrown.
    ///
    /// @throw RenderAPIException
    ///   Thrown if failed to create the swap chain and get back buffers.
    [[nodiscard]] InkExport auto newSwapChain(HWND          window,
                                              std::uint32_t numBuffers = 2,
                                              DXGI_FORMAT   format     = DXGI_FORMAT_R8G8B8A8_UNORM,
                                              bool          tearing    = true) -> SwapChain;

    /// @brief
    ///   Create a new command buffer.
    ///
    /// @throw RenderAPIException
    ///   Thrown if failed to create the new command buffer.
    [[nodiscard]] InkExport auto newCommandBuffer() -> CommandBuffer;

    friend class CommandBuffer;
    friend class ConstantBufferView;
    friend class RenderTargetView;
    friend class DepthStencilView;
    friend class Sampler;
    friend class SwapChain;
    friend class DynamicBufferAllocator;
    friend class DynamicDescriptorHeap;

private:
    /// @brief
    ///   Acquire and signal a new fence value. The fence value could be used to synchronize between
    ///   CPU and GPU.
    ///
    /// @return
    ///   The signaled fence value.
    [[nodiscard]] auto signalFence() const noexcept -> std::uint64_t {
        const std::uint64_t value = m_fenceValue.fetch_add(1U, std::memory_order_relaxed);
        m_commandQueue->Signal(m_fence.Get(), value);
        return value;
    }

    /// @brief
    ///   Block current thread until GPU complete all tasks before the signaled fence value.
    ///
    /// @param fenceValue
    ///   The fence value to be waited for.
    ///
    /// @throw SystemErrorException
    ///   Thrown if failed to create Win32 event object.
    auto sync(std::uint64_t fenceValue) const -> void;

    /// @brief
    ///   For internal usage. Acquire a new direct command allocator.
    ///
    /// @return
    ///   The new direct command allocator.
    ///
    /// @throw RenderAPIException
    ///   Thrown if failed to create new command allocator.
    [[nodiscard]] auto acquireCommandAllocator() -> ID3D12CommandAllocator *;

    /// @brief
    ///   For internal usage. Free the specified direct command allocator to reuse in the future.
    ///
    /// @param fenceValue
    ///   Fence value that indicates when the freed command allocator could be reused.
    /// @param allocator
    ///   The command allocator to be freed.
    auto releaseCommandAllocator(std::uint64_t           fenceValue,
                                 ID3D12CommandAllocator *allocator) noexcept -> void;

    /// @brief
    ///   For internal usage. Acquire a new constant buffer view descriptor. Constant buffer view
    ///   descriptor could also be used as shader resource view descriptor and unordered access view
    ///   descriptor.
    ///
    /// @return
    ///   The constant buffer view descriptor pointer.
    ///
    /// @throw RenderAPIException
    ///   Thrown if failed to create new CBV/SRV/UAV descriptor heap.
    [[nodiscard]] auto acquireConstantBufferViewDescriptor() -> std::size_t;

    /// @brief
    ///   For internal usage. Release the specified constant buffer view descriptor to reuse in the
    ///   future.
    ///
    /// @param ptr
    ///   The constant buffer view descriptor pointer to be released.
    auto releaseConstantBufferViewDescriptor(std::size_t ptr) noexcept -> void;

    /// @brief
    ///   For internal usage. Acquire a new sampler view descriptor.
    ///
    /// @return
    ///   The sampler view descriptor pointer.
    ///
    /// @throw RenderAPIException
    ///   Thrown if failed to create new sampler view descriptor heap.
    [[nodiscard]] auto acquireSamplerDescriptor() -> std::size_t;

    /// @brief
    ///   For internal usage. Release the specified sampler view descriptor to reuse in the future.
    ///
    /// @param ptr
    ///   The sampler view descriptor pointer to be released.
    auto releaseSamplerDescriptor(std::size_t ptr) noexcept -> void;

    /// @brief
    ///   For internal usage. Acquire a new render target view descriptor.
    ///
    /// @return
    ///   The render target view descriptor pointer.
    ///
    /// @throw RenderAPIException
    ///   Thrown if failed to create new render target view descriptor heap.
    [[nodiscard]] auto acquireRenderTargetViewDescriptor() -> std::size_t;

    /// @brief
    ///   For internal usage. Release the specified render target view descriptor to reuse in the
    ///   future.
    ///
    /// @param ptr
    ///   The render target view descriptor pointer to be released.
    auto releaseRenderTargetViewDescriptor(std::size_t ptr) noexcept -> void;

    /// @brief
    ///   For internal usage. Acquire a new depth stencil view descriptor.
    ///
    /// @return
    ///   The depth stencil view descriptor pointer.
    ///
    /// @throw RenderAPIException
    ///   Thrown if failed to create new depth stencil view descriptor heap.
    [[nodiscard]] auto acquireDepthStencilViewDescriptor() -> std::size_t;

    /// @brief
    ///   For internal usage. Release the specified depth stencil view descriptor to reuse in the
    ///   future.
    ///
    /// @param ptr
    ///   The depth stencil view descriptor pointer to be released.
    auto releaseDepthStencilViewDescriptor(std::size_t ptr) noexcept -> void;

    /// @brief
    ///   For internal usage. Allocate a new shader-visible CBV/SRV/UAV descriptor heap.
    ///
    /// @return
    ///   A new shader-visible CBV/SRV/UAV descriptor heap.
    ///
    /// @throw RenderAPIException
    ///   Thrown if failed to create new shader-visible CBV/SRV/UAV descriptor heap.
    [[nodiscard]] auto acquireDynamicViewHeap() -> ID3D12DescriptorHeap *;

    /// @brief
    ///   Free shader-visible CBV/SRV/UAV descriptor heaps.
    ///
    /// @param fenceValue
    ///   A fence value that indicates when these dynamic descriptor heaps could be reused.
    /// @param count
    ///   Number of shader-visible CBV/SRV/UAV descriptor heaps to be freed.
    /// @param heaps
    ///   Pointer to start of the descriptor heap array to be freed.
    auto releaseDynamicViewHeaps(std::uint64_t          fenceValue,
                                 std::size_t            count,
                                 ID3D12DescriptorHeap **heaps) noexcept -> void;

    /// @brief
    ///   For internal usage. Allocate a new shader-visible sampler descriptor heap.
    ///
    /// @return
    ///   A new shader-visible sampler descriptor heap.
    ///
    /// @throw RenderAPIException
    ///   Thrown if failed to create new shader-visible sampler descriptor heap.
    [[nodiscard]] auto acquireDynamicSamplerHeap() -> ID3D12DescriptorHeap *;

    /// @brief
    ///   Free shader-visible sampler descriptor heaps.
    ///
    /// @param fenceValue
    ///   A fence value that indicates when these dynamic descriptor heaps could be reused.
    /// @param count
    ///   Number of shader-visible sampler descriptor heaps to be freed.
    /// @param heaps
    ///   Pointer to start of the descriptor heap array to be freed.
    auto releaseDynamicSamplerHeaps(std::uint64_t          fenceValue,
                                    std::size_t            count,
                                    ID3D12DescriptorHeap **heaps) noexcept -> void;

    /// @brief
    ///   For @p DynamicBufferAllocator to use. Allocate a new dynamic upload buffer page.
    ///
    /// @param size
    ///   Expected size in byte of the new dynamic upload buffer page.
    ///
    /// @return
    ///   Pointer to the new dynamic buffer page.
    ///
    /// @throw RenderAPIException
    ///   Thrown if failed to create new dynamic buffer page.
    [[nodiscard]] auto acquireDynamicBufferPage(std::size_t size) -> DynamicBufferPage *;

    /// @brief
    ///   Free retired dynamic buffer pages.
    ///
    /// @param fenceValue
    ///   Fence value that indicates when the freed pages could be reused.
    /// @param count
    ///   Number of pages to be freed.
    /// @param pages
    ///   Array of dynamic buffer pages to be freed.
    auto releaseDynamicBufferPages(std::uint64_t       fenceValue,
                                   std::size_t         count,
                                   DynamicBufferPage **pages) noexcept -> void;

private:
    /// @brief
    ///   The DXGI factory that is used to initialize D3D12.
    Microsoft::WRL::ComPtr<IDXGIFactory6> m_dxgiFactory;

    /// @brief
    ///   The DXGI adapter that is used to create D3D12 device.
    Microsoft::WRL::ComPtr<IDXGIAdapter1> m_dxgiAdapter;

    /// @brief
    ///   The D3D12 virtual device.
    Microsoft::WRL::ComPtr<ID3D12Device5> m_device;

    /// @brief
    ///   Default direct command queue for this device.
    Microsoft::WRL::ComPtr<ID3D12CommandQueue> m_commandQueue;

    /// @brief
    ///   Default fence that is used to synchnorize between CPU and GPU.
    Microsoft::WRL::ComPtr<ID3D12Fence1> m_fence;

    /// @brief
    ///   Next fence value to be signaled.
    mutable std::atomic_uint64_t m_fenceValue;

    /// @brief
    ///   Direct command allocator pool.
    concurrency::concurrent_queue<ID3D12CommandAllocator *> m_allocatorPool;

    /// @brief
    ///   Freed direct command allocators.
    std::queue<std::pair<std::uint64_t, ID3D12CommandAllocator *>> m_freeAllocatorQueue;

    /// @brief
    ///   Mutex to protect free direct command allocator queue.
    mutable std::mutex m_freeAllocatorQueueMutex;

    /// @brief
    ///   CBV/SRV/UAV descriptor handle increment size.
    std::uint32_t m_constantBufferViewIncrementSize;

    /// @brief
    ///   Sampler descriptor handle increment size.
    std::uint32_t m_samplerViewIncrementSize;

    /// @brief
    ///   RTV descriptor handle increment size.
    std::uint32_t m_renderTargetViewIncrementSize;

    /// @brief
    ///   DSV descriptor handle increment size.
    std::uint32_t m_depthStencilViewIncrementSize;

    /// @brief
    ///   CPU descriptor heap pool.
    concurrency::concurrent_queue<ID3D12DescriptorHeap *> m_descriptorHeapPool;

    /// @brief
    ///   Free CBV/SRV/UAV descriptor handle queue.
    concurrency::concurrent_queue<std::size_t> m_freeConstantBufferViewQueue;

    /// @brief
    ///   Free sampler descriptor handle queue.
    concurrency::concurrent_queue<std::size_t> m_freeSamplerViewQueue;

    /// @brief
    ///   Free RTV CPU descriptor handle queue.
    concurrency::concurrent_queue<std::size_t> m_freeRenderTargetViewQueue;

    /// @brief
    ///   Free DSV CPU descriptor handle queue.
    concurrency::concurrent_queue<std::size_t> m_freeDepthStencilViewQueue;

    /// @brief
    ///   Current free CBV/SRV/UAV handle in the last free descriptor heap.
    std::size_t m_currentConstantBufferView;

    /// @brief
    ///   Current free sampler descriptor handle in the last free descriptor heap.
    std::size_t m_currentSamplerView;

    /// @brief
    ///   Current free render target view in the last free descriptor heap.
    std::size_t m_currentRenderTargetView;

    /// @brief
    ///   Current free depth stencil view in the last free descriptor heap.
    std::size_t m_currentDepthStencilView;

    /// @brief
    ///   Number of free CBV/SRV/UAVs in the last free descriptor heap.
    std::uint32_t m_freeConstantBufferViewCount;

    /// @brief
    ///   Number of free sampler views in the last free descriptor heap.
    std::uint32_t m_freeSamplerViewCount;

    /// @brief
    ///   Number of free render target views in the last free descriptor heap.
    std::uint32_t m_freeRenderTargetViewCount;

    /// @brief
    ///   Number of free depth stencil views in the last free descriptor heap.
    std::uint32_t m_freeDepthStencilViewCount;

    /// @brief
    ///   Mutex to protect CBV/SRV/UAV allocation.
    mutable std::mutex m_constantBufferViewAllocateMutex;

    /// @brief
    ///   Mutex to protect sampler view allocation.
    mutable std::mutex m_samplerViewAllocationMutex;

    /// @brief
    ///   Mutex to protect render target view allocation.
    mutable std::mutex m_renderTargetViewAllocationMutex;

    /// @brief
    ///   Mutex to protect depth stencil view allocation.
    mutable std::mutex m_depthStencilViewAllocationMutex;

    /// @brief
    ///   Freed shader visible CBV/SRV/UAV descriptor heaps.
    std::queue<std::pair<std::uint64_t, ID3D12DescriptorHeap *>> m_freeDynViewHeapQueue;

    /// @brief
    ///   Freed shader visible sampler descriptor heaps.
    std::queue<std::pair<std::uint64_t, ID3D12DescriptorHeap *>> m_freeDynSamplerHeapQueue;

    /// @brief
    ///   Mutex to protect retired dynamic CBV/SRV/UAV descriptor heaps.
    mutable std::mutex m_freeDynViewHeapQueueMutex;

    /// @brief
    ///   Mutex to protect retired dynamic sampler descriptor heaps.
    mutable std::mutex m_freeDynSamplerHeapQueueMutex;

    /// @brief
    ///   Dynamic buffer page pool.
    std::stack<DynamicBufferPage> m_dynBufferPagePool;

    /// @brief
    ///   Mutex to protect dynamic buffer page pool.
    mutable std::mutex m_dynBufferPagePoolMutex;

    /// @brief
    ///   Freed dynamic buffer pages.
    std::queue<std::pair<std::uint64_t, DynamicBufferPage *>> m_freeDynBufferPageQueue;

    /// @brief
    ///   Mutex to protect freed dynamic buffer pages.
    mutable std::mutex m_freeDynBufferPageQueueMutex;

    /// @brief
    ///   Freed dynamic buffer pages to be deleted.
    std::queue<std::pair<std::uint64_t, DynamicBufferPage *>> m_deletionDynBufferPageQueue;

    /// @brief
    ///   Mutex to protect freed dynamic buffer pages to be deleted.
    mutable std::mutex m_deletionDynBufferPageQueueMutex;
};

} // namespace ink

#pragma once

#include <concurrent_queue.h>
#include <d3d12.h>
#include <dxgi1_6.h>
#include <wrl/client.h>

#include <mutex>
#include <queue>

namespace ink {

/// @brief
///   D3D12 render device class. All render objects are created from the default render device.
class RenderDevice {
public:
    /// @brief
    ///   Initialize and create render device.
    /// @note
    ///   Render device is a huge object and may take a long time to construct and destruct.
    ///   Assertion is used to deal with render API errors. Error messages will be written to
    ///   default logger.
    RenderDevice() noexcept;

    /// @brief
    ///   Release all D3D12 resources and destroy this render device.
    /// @note
    ///   Destroying render device requires synchronizing with GPU which may take a long time to
    ///   make sure that all resources could be released safely.
    ~RenderDevice() noexcept;

    /// @brief
    ///   Get the DXGI factory that is used to initialize this render device.
    ///
    /// @return
    ///   The DXGI factory that is used to initialize this render device.
    [[nodiscard]]
    auto factory() const noexcept -> IDXGIFactory6 * {
        return m_factory.Get();
    }

    /// @brief
    ///   Get the DXGI adapter that is used to create the D3D12 device.
    ///
    /// @return
    ///   The DXGI adapter that is used to create the D3D12 device.
    [[nodiscard]]
    auto adapter() const noexcept -> IDXGIAdapter1 * {
        return m_adapter.Get();
    }

    /// @brief
    ///   Get D3D12 device of this render device.
    ///
    /// @return
    ///   The D3D12 device of this render device.
    [[nodiscard]]
    auto device() const noexcept -> ID3D12Device5 * {
        return m_device.Get();
    }

    /// @brief
    ///   Get default direct command queue of this render device.
    ///
    /// @return
    ///   Default direct command queue of this render device.
    [[nodiscard]]
    auto commandQueue() const noexcept -> ID3D12CommandQueue * {
        return m_cmdQueue.Get();
    }

    /// @brief
    ///   Acquire and signal a new fence value.
    /// @remark
    ///   Fence value could be used to synchronize CPU and GPU operations. See MSDN for details.
    ///
    /// @return
    ///   The new signaled fence value.
    [[nodiscard]]
    auto signalFence() const noexcept -> std::uint64_t {
        const auto value = m_nextFenceValue.fetch_add(1U, std::memory_order_relaxed);
        m_cmdQueue->Signal(m_fence.Get(), value);
        return value;
    }

    /// @brief
    ///   Checks if the specified fence value (sync point) has been reached by GPU.
    ///
    /// @param fenceValue
    ///   The fence value (sync point) to be checked.
    /// @return
    ///   A boolean value that indicates whether the specified fence value has been reached by GPU.
    /// @retval true
    ///   The specified fence value has been reached by GPU.
    /// @retval false
    ///   The specified fence value has not been reached by GPU.
    [[nodiscard]]
    auto isFenceReached(std::uint64_t fenceValue) const noexcept -> bool {
        return (fenceValue <= m_fence->GetCompletedValue());
    }

    /// @brief
    ///   Block current thread until GPU reaches the specified fence value (sync point).
    ///
    /// @param fenceValue
    ///   The fence value to be waited for.
    auto sync(std::uint64_t fenceValue) const noexcept -> void;

    /// @brief
    ///   Block current thread until all tasks submitted to GPU before this method call to be
    ///   finished.
    auto sync() const noexcept -> void {
        this->sync(this->signalFence());
    }

    /// @brief
    ///   Acquire a new direct command allocator.
    /// @note
    ///   Errors are handled with assertion and the return value is guaranteed to be valid.
    ///
    /// @return
    ///   A new direct command allocator.
    [[nodiscard]]
    auto newCommandAllocator() noexcept -> ID3D12CommandAllocator *;

    /// @brief
    ///   Free the specified direct command allocator to reuse in the future.
    ///
    /// @param fenceValue
    ///   Fence value that indicates when the freed command allocator could be reused.
    /// @param allocator
    ///   The command allocator to be freed.
    auto freeCommandAllocator(std::uint64_t fenceValue, ID3D12CommandAllocator *allocator) noexcept
        -> void;

    /// @brief
    ///   Checks if this render device supports DirectX ray tracing.
    ///
    /// @return
    ///   A boolean value that indicates whether this render device supports DXR.
    /// @retval true
    ///   This render device supports DXR.
    /// @retval false
    ///   This render device does not support DXR.
    [[nodiscard]]
    auto supportRayTracing() const noexcept -> bool;

    /// @brief
    ///   Checks if the specified pixel format supports unordered access.
    ///
    /// @param format
    ///   The pixel format to be checked.
    /// @return
    ///   A boolean value that indicates whether the specified pixel format supports unrodered
    ///   access.
    /// @retval true
    ///   The specified pixel format supports unordered access.
    /// @retval false
    ///   The specified pixel format does not support unordered access.
    [[nodiscard]]
    auto supportUnorderedAccess(DXGI_FORMAT format) const noexcept -> bool;

    /// @brief
    ///   Allocate a new constant buffer view CPU descriptor.
    /// @remark
    ///   Constant buffer view CPU descriptor could also be used as shader resource view descriptor
    ///   and unordered access view descriptor.
    ///
    /// @return
    ///   A D3D12 CPU descriptor handle to a free constant buffer view.
    [[nodiscard]]
    auto newConstantBufferView() noexcept -> D3D12_CPU_DESCRIPTOR_HANDLE;

    /// @brief 
    ///   Free a constant buffer view CPU descriptor.
    /// 
    /// @param handle 
    ///   CPU handle to the constant buffer view CPU descriptor to be freed.
    auto freeConstantBufferView(D3D12_CPU_DESCRIPTOR_HANDLE handle) noexcept -> void;

    /// @brief 
    ///   Allocate a new shader resource view CPU descriptor.
    /// @note
    ///   This method is an alias of @p RenderDevice::newConstantBufferView().
    /// 
    /// @return
    ///   A D3D12 CPU descriptor handle to a free shader resource view.
    [[nodiscard]]
    auto newShaderResourceView() noexcept -> D3D12_CPU_DESCRIPTOR_HANDLE {
        return this->newConstantBufferView();
    }

    /// @brief 
    ///   Free a shader resource view CPU descriptor.
    /// @note
    ///   This method is an alias of @p RenderDevice::freeConstantBufferView().
    /// 
    /// @param handle 
    ///   CPU handle to the shader resource view CPU descriptor to be freed.
    auto freeShaderResourceView(D3D12_CPU_DESCRIPTOR_HANDLE handle) noexcept -> void {
        this->freeConstantBufferView(handle);
    }

    /// @brief
    ///   Allocate a new unordered access view CPU descriptor.
    /// @note
    ///   This method is an alias of @p RenderDevice::newConstantBufferView().
    ///
    /// @return
    ///   A D3D12 CPU descriptor handle to a free unordered access view.
    [[nodiscard]]
    auto newUnorderedAccessView() noexcept -> D3D12_CPU_DESCRIPTOR_HANDLE {
        return this->newConstantBufferView();
    }

    /// @brief
    ///   Free an unordered access view CPU descriptor.
    /// @note
    ///   This method is an alias of @p RenderDevice::freeConstantBufferView().
    ///
    /// @param handle
    ///   CPU handle to the unordered access view CPU descriptor to be freed.
    auto freeUnorderedAccessView(D3D12_CPU_DESCRIPTOR_HANDLE handle) noexcept -> void {
        this->freeConstantBufferView(handle);
    }

    /// @brief 
    ///   Allocate a new sampler CPU descriptor.
    /// 
    /// @return
    ///   A D3D12 CPU descriptor handle to a free sampler view.
    [[nodiscard]]
    auto newSamplerView() noexcept -> D3D12_CPU_DESCRIPTOR_HANDLE;

    /// @brief
    ///   Free a sampler CPU descriptor.
    ///
    /// @param handle
    ///   CPU handle to the sampler CPU descriptor to be freed.
    auto freeSamplerView(D3D12_CPU_DESCRIPTOR_HANDLE handle) noexcept -> void;

    /// @brief
    ///   Allocate a new render target view CPU descriptor.
    ///
    /// @return
    ///   A D3D12 CPU descriptor handle to a free render target view.
    [[nodiscard]]
    auto newRenderTargetView() noexcept -> D3D12_CPU_DESCRIPTOR_HANDLE;

    /// @brief
    ///   Free a render target view CPU descriptor.
    ///
    /// @param handle
    ///   CPU handle to the render target view CPU descriptor to be freed.
    auto freeRenderTargetView(D3D12_CPU_DESCRIPTOR_HANDLE handle) noexcept -> void;

    /// @brief
    ///   Allocate a new depth stencil view CPU descriptor.
    ///
    /// @return
    ///   A D3D12 CPU descriptor handle to a free depth stencil view.
    [[nodiscard]]
    auto newDepthStencilView() noexcept -> D3D12_CPU_DESCRIPTOR_HANDLE;

    /// @brief
    ///   Free a depth stencil view CPU descriptor.
    ///
    /// @param handle
    ///   CPU handle to the depth stencil view CPU descriptor to be freed.
    auto freeDepthStencilView(D3D12_CPU_DESCRIPTOR_HANDLE handle) noexcept -> void;

    /// @brief
    ///   Get render device singleton instance.
    /// @note
    ///   This function is thread-safe. The singleton instance will be initialized when this
    ///   function is called for the first time.
    ///
    /// @return
    ///   Reference to the render device singleton instance.
    [[nodiscard]]
    static auto singleton() noexcept -> RenderDevice &;

private:
    /// @brief
    ///   The DXGI factory that is used to initialize D3D12.
    Microsoft::WRL::ComPtr<IDXGIFactory6> m_factory;

    /// @brief
    ///   The DXGI adapter that is used to create the D3D12 device.
    Microsoft::WRL::ComPtr<IDXGIAdapter1> m_adapter;

    /// @brief
    ///   The D3D12 virtual device.
    Microsoft::WRL::ComPtr<ID3D12Device5> m_device;

    /// @brief
    ///   D3D12 info queue for handling debug messages. Windows 11 only.
    Microsoft::WRL::ComPtr<ID3D12InfoQueue1> m_infoQueue;

    /// @brief
    ///   Default direct command queue for this device.
    Microsoft::WRL::ComPtr<ID3D12CommandQueue> m_cmdQueue;

    /// @brief
    ///   Default fence that is used to synchnorize the default direct command queue.
    Microsoft::WRL::ComPtr<ID3D12Fence1> m_fence;

    /// @brief
    ///   Next fence value to be signaled.
    mutable std::atomic_uint64_t m_nextFenceValue;

    /// @brief
    ///   Direct command allocator pool.
    ::Concurrency::concurrent_queue<ID3D12CommandAllocator *> m_allocatorPool;

    /// @brief
    ///   Free direct command allocators to be reused.
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
    ::Concurrency::concurrent_queue<ID3D12DescriptorHeap *> m_descriptorHeapPool;

    /// @brief
    ///   Free CBV/SRV/UAV CPU descriptor handle queue.
    ::Concurrency::concurrent_queue<D3D12_CPU_DESCRIPTOR_HANDLE> m_freeConstantBufferViewQueue;

    /// @brief
    ///   Free sampler descriptor handle queue.
    ::Concurrency::concurrent_queue<D3D12_CPU_DESCRIPTOR_HANDLE> m_freeSamplerViewQueue;

    /// @brief
    ///   Free RTV CPU descriptor handle queue.
    ::Concurrency::concurrent_queue<D3D12_CPU_DESCRIPTOR_HANDLE> m_freeRenderTargetViewQueue;

    /// @brief
    ///   Free DSV CPU descriptor handle queue.
    ::Concurrency::concurrent_queue<D3D12_CPU_DESCRIPTOR_HANDLE> m_freeDepthStencilViewQueue;

    /// @brief
    ///   Current CBV/SRV/UAV handle in the last free descriptor heap.
    D3D12_CPU_DESCRIPTOR_HANDLE m_currentConstantBufferView;

    /// @brief
    ///   Current free sampler descriptor handle in the last free descriptor heap.
    D3D12_CPU_DESCRIPTOR_HANDLE m_currentSamplerView;

    /// @brief
    ///   Current free render target view in the last free descriptor heap.
    D3D12_CPU_DESCRIPTOR_HANDLE m_currentRenderTargetView;

    /// @brief
    ///   Current free depth stencil view in the last free descriptor heap.
    D3D12_CPU_DESCRIPTOR_HANDLE m_currentDepthStencilView;

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
};

} // namespace ink

#pragma once

#include "pipeline.hpp"
#include "resource.hpp"

#include <vector>

namespace ink {

class DynamicBufferPage final : public GpuResource {
private:
    /// @brief
    ///   For internal usage. Create a new dynamic buffer page.
    ///
    /// @param device
    ///   D3D12 device that is used to create this dynamic buffer page.
    /// @param size
    ///   Expected size in byte of this dynamic buffer page.
    ///
    /// @throw RenderAPIException
    ///   Thrown if failed to create the dynamic buffer page.
    DynamicBufferPage(ID3D12Device *device, std::size_t size);

    friend class RenderDevice;

public:
    /// @brief
    ///   Copy constructor of dynamic buffer page is disabled.
    DynamicBufferPage(const DynamicBufferPage &) = delete;

    /// @brief
    ///   Move constructor of dynamic buffer page.
    ///
    /// @param other
    ///   The dynamic buffer page to be moved. The moved dynamic buffer page will be invalidated.
    InkExport DynamicBufferPage(DynamicBufferPage &&other) noexcept;

    /// @brief
    ///   Destroy this dynamic buffer page.
    InkExport ~DynamicBufferPage() noexcept override;

    /// @brief
    ///   Copy assignment of dynamic buffer page is disabled.
    auto operator=(const DynamicBufferPage &) = delete;

    /// @brief
    ///   Move assignment of dynamic buffer page.
    ///
    /// @param other
    ///   The dynamic buffer page to be moved. The moved dynamic buffer page will be invalidated.
    InkExport auto operator=(DynamicBufferPage &&other) noexcept -> DynamicBufferPage &;

    /// @brief
    ///   Get size in byte of this dynamic buffer page.
    ///
    /// @return
    ///   Size in byte of this dynamic buffer page.
    [[nodiscard]] auto size() const noexcept -> std::size_t { return m_size; }

    /// @brief
    ///   Map this dynamic buffer page to CPU memory. The virtual memory is writable only.
    ///
    /// @tparam T
    ///   Type of the pointer to be mapped.
    ///
    /// @return
    ///   Pointer to start of the mapped memory.
    template <typename T>
    [[nodiscard]] auto map() const noexcept -> T * {
        return static_cast<T *>(m_data);
    }

    /// @brief
    ///   Get GPU virtual address to start of this dynamic buffer page.
    ///
    /// @return
    ///   GPU virtual address to start of this dynamic buffer page.
    [[nodiscard]] auto gpuAddress() const noexcept -> std::uint64_t { return m_gpuAddress; }

private:
    /// @brief
    ///   Size in byte of this command buffer.
    std::size_t m_size;

    /// @brief
    ///   Pointer to start of this dynamic buffer page.
    void *m_data;

    /// @brief
    ///   GPU virtual address to start of this dynamic buffer page.
    std::uint64_t m_gpuAddress;
};

struct DynamicBufferAllocation {
    /// @brief
    ///   The GPU resource that this allocation belongs to.
    GpuResource *resource;

    /// @brief
    ///   Size in byte of this allocation.
    std::size_t size;

    /// @brief
    ///   Offset from start of the GPU resource. This offset has already been applied to the CPU
    ///   address and GPU address. Do not apply this offset again.
    std::size_t offset;

    /// @brief
    ///   CPU pointer to start of this allocation.
    void *data;

    /// @brief
    ///   GPU virtual address to start of this allocation.
    std::uint64_t gpuAddress;
};

class DynamicBufferAllocator {
private:
    /// @brief
    ///   Create a new dynamic buffer allocator.
    ///
    /// @param renderDevice
    ///   The render device that is used to manage buffer pages.
    DynamicBufferAllocator(RenderDevice &renderDevice) noexcept;

    friend class CommandBuffer;

public:
    /// @brief
    ///   Create an empty dynamic buffer allocator.
    InkExport DynamicBufferAllocator() noexcept;

    /// @brief
    ///   Copy constructor of dynamic buffer allocator is disabled.
    DynamicBufferAllocator(const DynamicBufferAllocator &) = delete;

    /// @brief
    ///   Move constructor of dynamic buffer allocator.
    ///
    /// @param other
    ///   The dynamic buffer allocator to be moved. The moved dynamic buffer allocator will be
    ///   invalidated.
    InkExport DynamicBufferAllocator(DynamicBufferAllocator &&other) noexcept;

    /// @brief
    ///   Destroy this dynamic buffer allocator and free all buffers.
    InkExport ~DynamicBufferAllocator() noexcept;

    /// @brief
    ///   Copy assignment of dynamic buffer allocator is disabled.
    auto operator=(const DynamicBufferAllocator &) = delete;

    /// @brief
    ///   Move assignment of dynamic buffer allocator.
    ///
    /// @param other
    ///   The dynamic buffer allocator to be moved. The moved dynamic buffer allocator will be
    ///   invalidated.
    InkExport auto operator=(DynamicBufferAllocator &&other) noexcept -> DynamicBufferAllocator &;

    /// @brief
    ///   Allocate a new buffer from this allocator.
    ///
    /// @param size
    ///   Expected size in byte of this dynamic upload buffer.
    /// @param alignment
    ///   Expected alignment in byte of this dynamic upload buffer. This value must be power of 2
    ///   and greater than 256.
    ///
    /// @return
    ///   The temporary upload buffer.
    [[nodiscard]] InkExport auto allocate(std::size_t size, std::size_t alignment = 256U)
        -> DynamicBufferAllocation;

    /// @brief
    ///   Free all retired buffers.
    ///
    /// @param fenceValue
    ///   A fence value that indicates when the retired buffers could be reused.
    InkExport auto reset(std::uint64_t fenceValue) noexcept -> void;

private:
    /// @brief
    ///   The render device that is used to manage buffer pages.
    RenderDevice *m_renderDevice;

    /// @brief
    ///   Current offset from start of this page.
    std::size_t m_offset;

    /// @brief
    ///   Current upload buffer page. This is type-erased pointer.
    DynamicBufferPage *m_page;

    /// @brief
    ///   Retired buffer pages.
    std::vector<DynamicBufferPage *> m_retiredPages;
};

class CommandBuffer {
private:
    /// @brief
    ///   For internal usage. Create a new command buffer.
    ///
    /// @param renderDevice
    ///   The render device that is used to create this command buffer.
    /// @param device
    ///   The D3D12 device that is used to create this command buffer.
    ///
    /// @throw RenderAPIException
    ///   Thrown if failed to create new command list.
    CommandBuffer(RenderDevice &renderDevice, ID3D12Device5 *device);

    friend class RenderDevice;

public:
    /// @brief
    ///   Create an empty command buffer.
    InkExport CommandBuffer() noexcept;

    /// @brief
    ///   Copy constructor of command buffer is disabled.
    CommandBuffer(const CommandBuffer &) = delete;

    /// @brief
    ///   Move constructor of command buffer.
    ///
    /// @param other
    ///   The command buffer to be moved. The moved command buffer will be invalidated.
    InkExport CommandBuffer(CommandBuffer &&other) noexcept;

    /// @brief
    ///   Destroy this command buffer.
    InkExport ~CommandBuffer() noexcept;

    /// @brief
    ///   Copy assignment of command buffer is disabled.
    auto operator=(const CommandBuffer &) = delete;

    /// @brief
    ///   Move assignment of command buffer.
    ///
    /// @param other
    ///   The command buffer to be moved. The moved command buffer will be invalidated.
    InkExport auto operator=(CommandBuffer &&other) noexcept -> CommandBuffer &;

    /// @brief
    ///   Submit this command buffer to start executing on GPU. This method will automatically reset
    ///   this command buffer once submitted tasks to GPU.
    ///
    /// @throw RenderAPIException
    ///   Thrown if failed to reset this command buffer.
    InkExport auto submit() -> void;

    /// @brief
    ///   Reset this command buffer and clean up all recorded commands.
    ///
    /// @throw RenderAPIException
    ///   Thrown if failed to acquire new resources.
    InkExport auto reset() -> void;

    /// @brief
    ///   Wait for last submission to be completed.
    ///
    /// @throw SystemErrorException
    ///   Thrown if failed to create synchronization event handle.
    InkExport auto waitForComplete() const -> void;

    /// @brief
    ///   Transition the specified GPU resource to new resource state.
    ///
    /// @param resource
    ///   The resource to be transitioned.
    /// @param newState
    ///   Expected resource state to be transitioned.
    InkExport auto transition(GpuResource &resource, D3D12_RESOURCE_STATES newState) noexcept
        -> void;

private:
    /// @brief
    ///   The render device that this command buffer is created from.
    RenderDevice *m_renderDevice;

    /// @brief
    ///   D3D12 direct command list.
    Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList4> m_cmdList;

    /// @brief
    ///   Command allocator for current command list.
    ID3D12CommandAllocator *m_allocator;

    /// @brief
    ///   Fence value that indicates when the last submittion will be finished.
    std::uint64_t m_lastSubmitFence;

    /// @brief
    ///   Temporary buffer allocator for this command buffer.
    DynamicBufferAllocator m_bufferAllocator;

    /// @brief
    ///   Current graphics root signature.
    RootSignature *m_graphicsRootSignature;

    /// @brief
    ///   Current compute root signature.
    RootSignature *m_computeRootSignature;

    /// @brief
    ///   Dynamic descriptor heap, for both CBV/SRV/UAV heap and sampler heap.
    DynamicDescriptorHeap m_dynamicDescriptorHeap;
};

} // namespace ink

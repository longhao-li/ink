#pragma once

#include "pipeline.hpp"
#include "resource.hpp"

#include <cstring>
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

enum class LoadAction {
    Discard  = D3D12_RENDER_PASS_BEGINNING_ACCESS_TYPE_DISCARD,
    Load     = D3D12_RENDER_PASS_BEGINNING_ACCESS_TYPE_PRESERVE,
    Clear    = D3D12_RENDER_PASS_BEGINNING_ACCESS_TYPE_CLEAR,
    DontCare = D3D12_RENDER_PASS_BEGINNING_ACCESS_TYPE_NO_ACCESS,
};

enum class StoreAction {
    Discard  = D3D12_RENDER_PASS_ENDING_ACCESS_TYPE_DISCARD,
    Store    = D3D12_RENDER_PASS_ENDING_ACCESS_TYPE_PRESERVE,
    DontCare = D3D12_RENDER_PASS_ENDING_ACCESS_TYPE_NO_ACCESS,
};

struct RenderPass {
    /// @brief
    ///   Number of render targets to be set for this render pass. Must be less than or equal to 8.
    std::uint32_t renderTargetCount;

    /// @brief
    ///   Render targets to be set for this render pass.
    struct RenderTarget {
        ColorBuffer          *renderTarget;
        LoadAction            loadAction;
        StoreAction           storeAction;
        D3D12_RESOURCE_STATES stateBefore;
        D3D12_RESOURCE_STATES stateAfter;
    } renderTargets[8];

    /// @brief
    ///   Depth target to be set for this render pass. nullable.
    struct DepthTarget {
        DepthBuffer          *depthTarget;
        LoadAction            depthLoadAction;
        StoreAction           depthStoreAction;
        LoadAction            stencilLoadAction;
        StoreAction           stencilStoreAction;
        D3D12_RESOURCE_STATES stateBefore;
        D3D12_RESOURCE_STATES stateAfter;
    } depthTarget;
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

    /// @brief
    ///   Begin a new render pass.
    ///
    /// @param renderPass
    ///   The render pass to be started.
    InkExport auto beginRenderPass(const RenderPass &renderPass) noexcept -> void;

    /// @brief
    ///   End current render pass. There must be a render pass started before calling this method.
    InkExport auto endRenderPass() noexcept -> void;

    /// @brief
    ///   Copy all data from @p src to @p dst.
    ///
    /// @param src
    ///   Source resource to be copied from.
    /// @param dst
    ///   Destination resource to be copied to.
    InkExport auto copy(GpuResource &src, GpuResource &dst) noexcept -> void;

    /// @brief
    ///   Copy buffer data from one GPU resource to another one.
    ///
    /// @param[in] src
    ///   Source buffer to be copied from.
    /// @param srcOffset
    ///   Offset from start of @p src to start the copy operation.
    /// @param[in] dst
    ///   Destination buffer to be copied to.
    /// @param dstOffset
    ///   Offset from start of @p dest to store the copied data.
    /// @param size
    ///   Size in byte of data to be copied.
    InkExport auto copyBuffer(GpuResource &src,
                              std::size_t  srcOffset,
                              GpuResource &dst,
                              std::size_t  dstOffset,
                              std::size_t  size) noexcept -> void;

    /// @brief
    ///   Copy data from system memory to the specified GPU buffer.
    ///
    /// @param src
    ///   Pointer to start of data to be copied.
    /// @param dst
    ///   The destination buffer to be copied to.
    /// @param dstOffset
    ///   Offset from start of @p dest to store the copied data.
    /// @param size
    ///   Size in byte of data to be copied.
    ///
    /// @throw RenderAPIException
    ///   Thrown if failed to allocate temporary upload buffer.
    InkExport auto
    copyBuffer(const void *src, GpuResource &dst, std::size_t dstOffset, std::size_t size) -> void;

    /// @brief
    ///   Copy data from system memory to texture.
    ///
    /// @param src
    ///   Source data to be copied from.
    /// @param srcFormat
    ///   Format of the source image. Only very limited formats support converting by this method.
    /// @param srcRowPitch
    ///   Texture row size in byte of source data.
    /// @param width
    ///   Width in pixel of the texture subresource.
    /// @param height
    ///   Height in pixel of the texture subresource.
    /// @param dst
    ///   The texture resource to be copied to.
    /// @param subresource
    ///   Subresource index of @p dst to be copied to. Subresource could be considered as mipmap
    ///   level for 2D textures.
    ///
    /// @throw RenderAPIException
    ///   Thrown if failed to allocate temporary upload buffer.
    InkExport auto copyTexture(const void   *src,
                               DXGI_FORMAT   srcFormat,
                               std::size_t   srcRowPitch,
                               std::uint32_t width,
                               std::uint32_t height,
                               PixelBuffer  &dst,
                               std::uint32_t subresource) -> void;

    /// @brief
    ///   Set graphics root signature for current command buffer.
    ///
    /// @param rootSig
    ///   The root signature to be set for graphics pipeline.
    InkExport auto setGraphicsRootSignature(RootSignature &rootSig) noexcept -> void;

    /// @brief
    ///   Set compute root signature for current command buffer.
    ///
    /// @param rootSig
    ///   The root signature to be set for compute pipeline.
    InkExport auto setComputeRootSignature(RootSignature &rootSig) noexcept -> void;

    /// @brief
    ///   Bind a descriptor to the specified graphics descriptor table. Both CBV/SRV/UAV and sampler
    ///   are supported.
    ///
    /// @param rootParam
    ///   Root parameter index of the descriptor table.
    /// @param offset
    ///   Offset from start of the root descriptor table to set the descriptor handle.
    /// @param handle
    ///   The descriptor handle to be set.
    InkExport auto setGraphicsDescriptor(std::uint32_t rootParam,
                                         std::uint32_t offset,
                                         CpuDescriptor handle) noexcept -> void;

    /// @brief
    ///   Bind a descriptor to the specified compute descriptor table. Both CBV/SRV/UAV and sampler
    ///   are supported.
    ///
    /// @param rootParam
    ///   Root parameter index of the descriptor table.
    /// @param offset
    ///   Offset from start of the root descriptor table to set the descriptor handle.
    /// @param handle
    ///   The descriptor handle to be set.
    InkExport auto setComputeDescriptor(std::uint32_t rootParam,
                                        std::uint32_t offset,
                                        CpuDescriptor handle) noexcept -> void;

    /// @brief
    ///   Set a graphics root constant.
    ///
    /// @tparam T
    ///   Type of the constant value to be set.
    ///
    /// @param rootParam
    ///   Index of the root constant parameter to be set.
    /// @param offset
    ///   The offset, in 32-bit values, to set the constant in the root signature.
    /// @param value
    ///   The 32-bit constant value to be set.
    template <typename T>
    auto setGraphicsConstant(std::uint32_t rootParam, std::uint32_t offset, const T &value) noexcept
        -> void {
        static_assert(sizeof(T) == sizeof(std::uint32_t), "Element must be 4 bytes in size.");
        static_assert(std::is_trivially_copyable_v<T>, "Element must be trivially copyable.");
        UINT constant; // Avoid alignment problem.
        std::memcpy(&constant, &value, sizeof(UINT));
        m_cmdList->SetGraphicsRoot32BitConstant(rootParam, constant, offset);
    }

    /// @brief
    ///   Set a compute root constant.
    ///
    /// @tparam T
    ///   Type of the constant value to be set.
    ///
    /// @param rootParam
    ///   Index of the root constant parameter to be set.
    /// @param offset
    ///   The offset, in 32-bit values, to set the constant in the root signature.
    /// @param value
    ///   The 32-bit constant value to be set.
    template <typename T>
    auto setComputeConstant(std::uint32_t rootParam, std::uint32_t offset, const T &value) noexcept
        -> void {
        static_assert(sizeof(T) == sizeof(std::uint32_t), "Element must be 4 bytes in size.");
        static_assert(std::is_trivially_copyable_v<T>, "Element must be trivially copyable.");
        UINT constant; // Avoid alignment problem.
        std::memcpy(&constant, &value, sizeof(UINT));
        m_cmdList->SetComputeRoot32BitConstant(rootParam, constant, offset);
    }

    /// @brief
    ///   Set viewport for current graphics pipeline.
    /// @remark
    ///   D3D12 screen coordinate starts from top-left corner of screen.
    ///
    /// @tparam T0
    ///   Type of x to be set for this viewport.
    /// @tparam T1
    ///   Type of y to be set for this viewport.
    /// @tparam T2
    ///   Type of width to be set for this viewport.
    /// @tparam T3
    ///   Type of height to be set for this viewport.
    ///
    /// @param x
    ///   X coordinate from top-left corner of this viewport.
    /// @param y
    ///   Y coordinate from top-left corner of this viewport.
    /// @param width
    ///   Width of this viewport.
    /// @param height
    ///   Height of this viewport.
    /// @param zNear
    ///   Minimum depth of this viewport. Default value is 0.
    /// @param zFar
    ///   Maximum depth of this viewport. Default value is 1.
    template <typename T0, typename T1, typename T2, typename T3>
    auto setViewport(T0 x, T1 y, T2 width, T3 height, float zNear = 0, float zFar = 1.f) noexcept
        -> void {
        static_assert(std::is_arithmetic<T0>::value, "X must be arithmetic type.");
        static_assert(std::is_arithmetic<T1>::value, "Y must be arithmetic type.");
        static_assert(std::is_arithmetic<T2>::value, "Width must be arithmetic type.");
        static_assert(std::is_arithmetic<T3>::value, "Height must be arithmetic type.");

        const D3D12_VIEWPORT viewport{
            /* TopLeftX = */ static_cast<float>(x),
            /* TopLeftY = */ static_cast<float>(y),
            /* Width    = */ static_cast<float>(width),
            /* Height   = */ static_cast<float>(height),
            /* MinDepth = */ zNear,
            /* MaxDepth = */ zFar,
        };

        m_cmdList->RSSetViewports(1, &viewport);
    }

    /// @brief
    ///   Set scissor rectangle for this graphics pipeline.
    /// @remark
    ///   D3D12 screen coordinate starts from top-left corner of screen.
    ///
    /// @tparam T0
    ///   Type of x to be set for this viewport.
    /// @tparam T1
    ///   Type of y to be set for this viewport.
    /// @tparam T2
    ///   Type of width to be set for this viewport.
    /// @tparam T3
    ///   Type of height to be set for this viewport.
    ///
    /// @param x
    ///   X coordinate from top-left corner of this viewport.
    /// @param y
    ///   Y coordinate from top-left corner of this viewport.
    /// @param width
    ///   Width of this viewport.
    /// @param height
    ///   Height of this viewport.
    template <typename T0, typename T1, typename T2, typename T3>
    auto setScissorRect(T0 x, T1 y, T2 width, T3 height) noexcept -> void {
        static_assert(std::is_arithmetic<T0>::value, "X must be arithmetic type.");
        static_assert(std::is_arithmetic<T1>::value, "Y must be arithmetic type.");
        static_assert(std::is_arithmetic<T2>::value, "Width must be arithmetic type.");
        static_assert(std::is_arithmetic<T3>::value, "Height must be arithmetic type.");

        const D3D12_RECT rect{
            /* left   = */ static_cast<LONG>(x),
            /* top    = */ static_cast<LONG>(y),
            /* right  = */ static_cast<LONG>(x + width),
            /* bottom = */ static_cast<LONG>(y + height),
        };

        m_cmdList->RSSetScissorRects(1, &rect);
    }

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

    /// @brief
    ///   Current render pass. For cache only.
    RenderPass m_renderPass;
};

} // namespace ink

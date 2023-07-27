#pragma once

#include "ink/render/pipeline.h"
#include "ink/render/resource.h"

#include <vector>

namespace ink {

class RenderDevice;

class DynamicDescriptorHeap {
private:
    enum class CacheType {
        None = 0,
        CpuDescriptor,
        ConstantBufferView,
    };

    struct DescriptorCache {
#if defined(__clang__)
#    pragma clang diagnostic push
#    pragma clang diagnostic ignored "-Wgnu-anonymous-struct"
#elif defined(__GNUC__)
#    pragma GCC diagnostic push
#    pragma GCC diagnostic ignored "-Wpedantic"
#elif defined(_MSC_VER)
#    pragma warning(push)
#    pragma warning(disable : 4201)
#endif
        CacheType cacheType;
        union {
            D3D12_CPU_DESCRIPTOR_HANDLE     handle;
            D3D12_CONSTANT_BUFFER_VIEW_DESC constantBuffer;
        };
#if defined(__clang__)
#    pragma clang diagnostic pop
#elif defined(__GNUC__)
#    pragma GCC diagnostic pop
#elif defined(_MSC_VER)
#    pragma warning(pop)
#endif
    };

    struct DescriptorTableRange {
        std::uint16_t start;
        std::uint16_t count;
    };

public:
    /// @brief
    ///   Create an empty dynamic descriptor heap.
    InkApi DynamicDescriptorHeap() noexcept;

    /// @brief
    ///   Create a dynamic descriptor heap for the specified type of descriptors.
    ///
    /// @param descriptorType
    ///   Type of this dynamic descriptor type.
    InkApi explicit DynamicDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE descriptorType) noexcept;

    /// @brief
    ///   Copy constructor of dynamic descriptor heap is disabled.
    DynamicDescriptorHeap(const DynamicDescriptorHeap &) = delete;

    /// @brief
    ///   Copy assignment of dynamic descriptor heap is disabled.
    auto operator=(const DynamicDescriptorHeap &) = delete;

    /// @brief
    ///   Move constructor of dynamic descriptor heap.
    ///
    /// @param other
    ///   The dynamic descriptor heap to be moved from. The moved dynamic descriptor heap will be
    ///   invalidated.
    InkApi DynamicDescriptorHeap(DynamicDescriptorHeap &&other) noexcept;

    /// @brief
    ///   Copy assignment of dynamic descriptor heap is disabled.
    auto operator=(DynamicDescriptorHeap &&) = delete;

    /// @brief
    ///   Destroy this dynamic descriptor heap.
    InkApi ~DynamicDescriptorHeap() noexcept;

    /// @brief
    ///   Clean up all descriptors and root signatures.
    ///
    /// @param fenceValue
    ///   The fence value that indicates when the allocated descriptor heaps could be reused.
    InkApi auto reset(std::uint64_t fenceValue) noexcept -> void;

    /// @brief
    ///   Parse the specified graphics root signature and prepare shader-visible descriptors for it.
    ///
    /// @param rootSig
    ///   The root signature to be parsed.
    InkApi auto parseGraphicsRootSignature(const RootSignature &rootSig) noexcept -> void;

    /// @brief
    ///   Parse the specified compute root signature and prepare shader-visible descriptors for it.
    ///
    /// @param rootSig
    ///   The root signature to be parsed.
    InkApi auto parseComputeRootSignature(const RootSignature &rootSig) noexcept -> void;

    /// @brief
    ///   Bind the specified CPU descriptor handle to the specified root descriptor table.
    ///
    /// @param paramIndex
    ///   Root parameter index of the descriptor table. This method does not check whether the
    ///   specified root parameter is a valid descriptor table.
    /// @param offset
    ///   Offset from start of the descriptor table to set the descriptor handle. This method does
    ///   not check whether the offset is out of range.
    /// @param descriptor
    ///   The CPU descriptor to be bind.
    InkApi auto bindGraphicsDescriptor(std::uint32_t       paramIndex,
                                       std::uint32_t       offset,
                                       CpuDescriptorHandle descriptor) noexcept -> void;

    /// @brief
    ///   Bind a constant buffer view to the specified root descriptor table.
    ///
    /// @param paramIndex
    ///   Root parameter index of the descriptor table. This method does not check whether the
    ///   specified root parameter is a valid descriptor table.
    /// @param offset
    ///   Offset from start of the descriptor table to set the descriptor handle. This method does
    ///   not check whether the offset is out of range.
    /// @param desc
    ///   A description structure that describes how to create the constant buffer view.
    InkApi auto bindGraphicsDescriptor(std::uint32_t                          paramIndex,
                                       std::uint32_t                          offset,
                                       const D3D12_CONSTANT_BUFFER_VIEW_DESC &desc) noexcept
        -> void;

    /// @brief
    ///   Bind the specified CPU descriptor handle to the specified root descriptor table.
    ///
    /// @param paramIndex
    ///   Root parameter index of the descriptor table. This method does not check whether the
    ///   specified root parameter is a valid descriptor table.
    /// @param offset
    ///   Offset from start of the descriptor table to set the descriptor handle. This method does
    ///   not check whether the offset is out of range.
    /// @param descriptor
    ///   The CPU descriptor to be bind.
    InkApi auto bindComputeDescriptor(std::uint32_t       paramIndex,
                                      std::uint32_t       offset,
                                      CpuDescriptorHandle descriptor) noexcept -> void;

    /// @brief
    ///   Bind a constant buffer view to the specified root descriptor table.
    ///
    /// @param paramIndex
    ///   Root parameter index of the descriptor table. This method does not check whether the
    ///   specified root parameter is a valid descriptor table.
    /// @param offset
    ///   Offset from start of the descriptor table to set the descriptor handle. This method does
    ///   not check whether the offset is out of range.
    /// @param desc
    ///   A description structure that describes how to create the constant buffer view.
    InkApi auto bindComputeDescriptor(std::uint32_t                          paramIndex,
                                      std::uint32_t                          offset,
                                      const D3D12_CONSTANT_BUFFER_VIEW_DESC &desc) noexcept -> void;

    /// @brief
    ///   Upload all descriptors in the cached descriptor tables to shader-visible descriptor heaps
    ///   and bind the specified descriptor heap to current command list.
    ///
    /// @param cmdList
    ///   The command list that is used to bind the specified descriptor heap.
    InkApi auto submitGraphicsDescriptors(ID3D12GraphicsCommandList *cmdList) noexcept -> void;

    /// @brief
    ///   Upload all descriptors in the cached descriptor tables to shader-visible descriptor heaps
    ///   and bind the specified descriptor heap to current command list.
    ///
    /// @param cmdList
    ///   The command list that is used to bind the specified descriptor heap.
    InkApi auto submitComputeDescriptors(ID3D12GraphicsCommandList *cmdList) noexcept -> void;

    /// @brief
    ///   Get current dynamic descriptor heap. Mainly used for CommandBuffer to set descriptor heaps.
    /// 
    /// @return
    ///   Current dynamic descriptor heap.
    [[nodiscard]]
    auto dynamicDescriptorHeap() const noexcept -> ID3D12DescriptorHeap* {
        return m_currentHeap;
    }

private:
    /// @brief
    ///   The render device that is used to allocate and free dynamic descriptor heaps.
    RenderDevice *m_renderDevice;

    /// @brief
    ///   The D3D12 device that is used to create and copy descriptors.
    ID3D12Device5 *m_device;

    /// @brief
    ///   Descriptor type of this dynamic descriptor heap.
    D3D12_DESCRIPTOR_HEAP_TYPE m_heapType;

    /// @brief
    ///   Descriptor handle increment size.
    std::uint32_t m_descriptorSize;

    /// @brief
    ///   Current graphics root signature of this dynamic descriptor heap.
    const RootSignature *m_graphicsRootSignature;

    /// @brief
    ///   Current compute root signature of this dynamic descriptor heap.
    const RootSignature *m_computeRootSignature;

    /// @brief
    ///   Current shader-visible descriptor heap.
    ID3D12DescriptorHeap *m_currentHeap;

    /// @brief
    ///   Current shader-visible descriptor handle.
    DescriptorHandle m_currentHandle;

    /// @brief
    ///   Number of free descriptors in current shader-visible descriptor heap.
    std::uint32_t m_freeDescriptorCount;

    /// @brief
    ///   Retired descriptor heaps.
    std::vector<ID3D12DescriptorHeap *> m_retiredHeaps;

    /// @brief
    ///   Cached graphics descriptors.
    std::vector<DescriptorCache> m_graphicsDescriptors;

    /// @brief
    ///   Cached compute descriptors.
    std::vector<DescriptorCache> m_computeDescriptors;

    /// @brief
    ///   Cached descriptor table ranges for graphics root signature.
    DescriptorTableRange m_graphicsTableRange[64];

    /// @brief
    ///   Cached descriptor table ranges for compute root signature.
    DescriptorTableRange m_computeTableRange[64];
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
public:
    /// @brief
    ///   Create an empty dynamic buffer allocator.
    InkApi DynamicBufferAllocator() noexcept;

    /// @brief
    ///   Copy constructor of dynamic buffer allocator is disabled.
    DynamicBufferAllocator(const DynamicBufferAllocator &) = delete;

    /// @brief
    ///   Copy assignment of dynamic buffer allocator is disabled.
    auto operator=(const DynamicBufferAllocator &) = delete;

    /// @brief
    ///   Destroy this dynamic buffer allocator and free all buffers.
    InkApi ~DynamicBufferAllocator() noexcept;

    /// @brief
    ///   Allocate a temporary upload buffer.
    /// @note
    ///   Errors are handled with assertions.
    ///
    /// @param size
    ///   Expected size in byte of this temporary upload buffer. This value will always be aligned
    ///   up with 256 bytes.
    ///
    /// @return
    ///   The temporary upload buffer allocation.
    [[nodiscard]]
    InkApi auto newUploadBuffer(std::size_t size) noexcept -> DynamicBufferAllocation;

    /// @brief
    ///   Allocate a temporary unordered access buffer.
    /// @note
    ///   Errors are handled with assertions.
    ///
    /// @param size
    ///   Expected size in byte of this temporary unordered access buffer. This value will always be
    ///   aligned up with 256 bytes.
    ///
    /// @return
    ///   The temporary unordered access buffer allocation.
    [[nodiscard]]
    InkApi auto newUnorderedAccessBuffer(std::size_t size) noexcept -> DynamicBufferAllocation;

    /// @brief
    ///   Free all retired buffers.
    ///
    /// @param fenceValue
    ///   A fence value that indicates when the retired buffers could be reused.
    InkApi auto reset(std::uint64_t fenceValue) noexcept -> void;

private:
    /// @brief
    ///   Current upload buffer page. This is type-erased pointer.
    void *m_uploadPage;

    /// @brief
    ///   Current offset from start of current upload page.
    std::size_t m_uploadPageOffset;

    /// @brief
    ///   Current unordered access buffer page. This is type-erased pointer.
    void *m_unorderedAccessPage;

    /// @brief
    ///   Offset from start of current unordered access page.
    std::size_t m_unorderedAccessPageOffset;

    /// @brief
    ///   Retired buffer pages.
    std::vector<void *> m_retiredPages;
};

class CommandBuffer {
public:
    /// @brief
    ///   Create a new command buffer.
    /// @note
    ///   Errors are handled by assertions.
    InkApi CommandBuffer() noexcept;

    /// @brief
    ///   Destroy this command buffer.
    InkApi ~CommandBuffer() noexcept;

    /// @brief
    ///   Submit this command buffer to start executing on GPU.
    /// @note
    ///   This method will automatically reset current command buffer once submitted.
    InkApi auto submit() noexcept -> std::uint64_t;

    /// @brief
    ///   Reset this command buffer and clean up all recorded commands.
    InkApi auto reset() noexcept -> void;

    /// @brief
    ///   Wait for last submission to be finished on GPU.
    InkApi auto waitForComplete() const noexcept -> void;

    /// @brief
    ///   Transition the specified GPU resource to new resource state.
    ///
    /// @param[in, out] resource
    ///   The GPU resource to be transitioned.
    /// @param newState
    ///   The new resource state of the GPU resource.
    InkApi auto transition(GpuResource &resource, D3D12_RESOURCE_STATES newState) noexcept -> void;

    /// @brief
    ///   Insert a UAV resource barrier.
    ///
    /// @param[in] resource
    ///   The resource that this barrier is inserted for.
    InkApi auto unorderedAccessBarrier(GpuResource &resource) noexcept -> void;

    /// @brief
    ///   Copy all data from @p src to @p dst.
    ///
    /// @param[in] src
    ///   Source resource to be copied from.
    /// @param[in, out] dst
    ///   Destination resource to be copied to.
    InkApi auto copy(GpuResource &src, GpuResource &dst) noexcept -> void;

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
    InkApi auto copyBuffer(GpuResource &src,
                           std::size_t  srcOffset,
                           GpuResource &dst,
                           std::size_t  dstOffset,
                           std::size_t  size) noexcept -> void;

    /// @brief
    ///   Copy data from system memory to the specified GPU buffer.
    ///
    /// @param[in] src
    ///   Pointer to start of data to be copied.
    /// @param[in] dst
    ///   The destination buffer to be copied to.
    /// @param dstOffset
    ///   Offset from start of @p dest to store the copied data.
    /// @param size
    ///   Size in byte of data to be copied.
    InkApi auto
    copyBuffer(const void *src, GpuResource &dst, std::size_t dstOffset, std::size_t size) noexcept
        -> void;

    /// @brief
    ///   Copy data from system memory to texture.
    ///
    /// @param[in] src
    ///   Source data to be copied from.
    /// @param srcFormat
    ///   Format of the source image. Only very limited formats support converting by this method.
    /// @param srcRowPitch
    ///   Texture row size in byte of source data.
    /// @param width
    ///   Width in pixel of the texture subresource.
    /// @param height
    ///   Height in pixel of the texture subresource.
    /// @param[in] dst
    ///   The texture resource to be copied to.
    /// @param subresource
    ///   Subresource index of @p dst to be copied to. Subresource could be considered as mipmap
    ///   level for 2D textures.
    InkApi auto copyTexture(const void   *src,
                            DXGI_FORMAT   srcFormat,
                            std::size_t   srcRowPitch,
                            std::uint32_t width,
                            std::uint32_t height,
                            PixelBuffer  &dst,
                            std::uint32_t subresource) noexcept -> void;

    /// @brief
    ///   Set a single render target for current graphics pipeline without depth buffer.
    ///
    /// @param[in] renderTarget
    ///   The color buffer to be used as render target.
    InkApi auto setRenderTarget(ColorBuffer &renderTarget) noexcept -> void;

    /// @brief
    ///   Set a single render target with a depth buffer for current graphics pipeline.
    ///
    /// @param[in] renderTarget
    ///   The color buffer to be used as render target.
    /// @param[in] depthTarget
    ///   The depth buffer to be used as depth target. The depth read-only view will be used if @p
    ///   D3D12_RESOURCE_STATE_DEPTH_WRITE state is not set for the depth buffer.
    InkApi auto setRenderTarget(ColorBuffer &renderTarget, DepthBuffer &depthTarget) noexcept
        -> void;

    /// @brief
    ///   Set a list of render targets for current graphics pipeline without depth buffer.
    ///
    /// @param renderTargetCount
    ///   Number of render targets to be set. This value should not exceed @p
    ///   D3D12_SIMULTANEOUS_RENDER_TARGET_COUNT.
    /// @param[in] renderTargets
    ///   The render targets to be set.
    InkApi auto setRenderTargets(std::size_t   renderTargetCount,
                                 ColorBuffer **renderTargets) noexcept -> void;

    /// @brief
    ///   Set a list of render targets for current graphics pipeline.
    ///
    /// @param renderTargetCount
    ///   Number of render targets to be set. This value should not exceed @p
    ///   D3D12_SIMULTANEOUS_RENDER_TARGET_COUNT.
    /// @param[in] renderTargets
    ///   The render targets to be set.
    /// @param[in] depthTarget
    ///   The depth buffer to be used as depth target. The depth read-only view will be used if @p
    ///   D3D12_RESOURCE_STATE_DEPTH_WRITE state is not set for the depth buffer.
    InkApi auto setRenderTargets(std::size_t   renderTargetCount,
                                 ColorBuffer **renderTargets,
                                 DepthBuffer  &depthTarget) noexcept -> void;

    /// @brief
    ///   Clear a color buffer to its clear color.
    ///
    /// @param[in] colorBuffer
    ///   The color buffer to be cleared.
    auto clearColor(ColorBuffer &colorBuffer) noexcept -> void {
        const Color &color = colorBuffer.clearColor();
        m_cmdList->ClearRenderTargetView(colorBuffer.renderTargetView(), color.m_arr, 0, nullptr);
    }

    /// @brief
    ///   Clear a color buffer to the specified color.
    ///
    /// @param[in] colorBuffer
    ///   The color buffer to be cleared.
    /// @param color
    ///   Clear color to be used to clear the color buffer.
    auto clearColor(ColorBuffer &colorBuffer, const Color &color) noexcept -> void {
        m_cmdList->ClearRenderTargetView(colorBuffer.renderTargetView(), color.m_arr, 0, nullptr);
    }

    /// @brief
    ///   Clear depth value for the specified depth buffer. Stencil values will not be modified by
    ///   this method.
    ///
    /// @param[in] depthBuffer
    ///   The depth buffer to be cleared.
    auto clearDepth(DepthBuffer &depthBuffer) noexcept -> void {
        m_cmdList->ClearDepthStencilView(depthBuffer.depthStencilView(), D3D12_CLEAR_FLAG_DEPTH,
                                         depthBuffer.clearDepth(), 0, 0, nullptr);
    }

    /// @brief
    ///   Clear depth value for the specified depth buffer. Stencil values will not be modified by
    ///   this method.
    ///
    /// @param[in] depthBuffer
    ///   The depth buffer to be cleared.
    /// @param depth
    ///   The depth value to be used to clear the depth buffer.
    auto clearDepth(DepthBuffer &depthBuffer, float depth) noexcept -> void {
        m_cmdList->ClearDepthStencilView(depthBuffer.depthStencilView(), D3D12_CLEAR_FLAG_DEPTH,
                                         depth, 0, 0, nullptr);
    }

    /// @brief
    ///   Clear stencil value for the specified depth buffer. Depth values will not be modified by
    ///   this method.
    ///
    /// @param[in] depthBuffer
    ///   The depth buffer to be cleared.
    auto clearStencil(DepthBuffer &depthBuffer) noexcept -> void {
        m_cmdList->ClearDepthStencilView(depthBuffer.depthStencilView(), D3D12_CLEAR_FLAG_STENCIL,
                                         0, depthBuffer.clearStencil(), 0, nullptr);
    }

    /// @brief
    ///   Clear stencil value for the specified depth buffer. Depth values will not be modified by
    ///   this method.
    ///
    /// @param[in] depthBuffer
    ///   The depth buffer to be cleared.
    /// @param stencil
    ///   The stencil value to be used to clear this depth buffer.
    auto clearStencil(DepthBuffer &depthBuffer, std::uint8_t stencil) noexcept -> void {
        m_cmdList->ClearDepthStencilView(depthBuffer.depthStencilView(), D3D12_CLEAR_FLAG_STENCIL,
                                         0, stencil, 0, nullptr);
    }

    /// @brief
    ///   Clear depth and stencil value for the specified depth buffer.
    ///
    /// @param[in] depthBuffer
    ///   The depth buffer to be cleared.
    auto clearDepthStencil(DepthBuffer &depthBuffer) noexcept -> void {
        m_cmdList->ClearDepthStencilView(
            depthBuffer.depthStencilView(), D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL,
            depthBuffer.clearDepth(), depthBuffer.clearStencil(), 0, nullptr);
    }

    /// @brief
    ///   Clear depth and stencil value for the specified depth buffer.
    ///
    /// @param[in] depthBuffer
    ///   The depth buffer to be cleared.
    /// @param depth
    ///   The depth value to be used to clear the depth buffer.
    /// @param stencil
    ///   The stencil value to be used to clear this depth buffer.
    auto clearDepthStencil(DepthBuffer &depthBuffer, float depth, std::uint8_t stencil) noexcept
        -> void {
        m_cmdList->ClearDepthStencilView(depthBuffer.depthStencilView(),
                                         D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, depth,
                                         stencil, 0, nullptr);
    }

    /// @brief
    ///   Set graphics root signature for current command buffer.
    ///
    /// @param[in] rootSig
    ///   The root signature to be set for graphics pipeline.
    InkApi auto setGraphicsRootSignature(RootSignature &rootSig) noexcept -> void;

    /// @brief
    ///   Bind a CBV/SRV/UAV to the specified descriptor table.
    ///
    /// @param rootParam
    ///   Root parameter index of the CBV/SRV/UAV descriptor table.
    /// @param offset
    ///   Offset from start of the root descriptor table to set the descriptor handle.
    /// @param handle
    ///   The CBV/SRV/UAV descriptor handle to be set.
    auto setGraphicsView(std::uint32_t       rootParam,
                         std::uint32_t       offset,
                         CpuDescriptorHandle handle) noexcept -> void {
        m_dynamicViewHeap.bindGraphicsDescriptor(rootParam, offset, handle);
    }

    /// @brief
    ///   Bind a sampler to the specified descriptor table.
    ///
    /// @param rootParam
    ///   Root parameter index of the sampler descriptor table.
    /// @param offset
    ///   Offset from start of the root descriptor table to set the descriptor handle.
    /// @param handle
    ///   The sampler descriptor handle to be set.
    auto setGraphicsSampler(std::uint32_t       rootParam,
                            std::uint32_t       offset,
                            CpuDescriptorHandle handle) noexcept -> void {
        m_dynamicSamplerHeap.bindGraphicsDescriptor(rootParam, offset, handle);
    }

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
        static_assert(sizeof(T) == sizeof(std::uint32_t),
                      "Element must be actually 4 bytes in size.");
        static_assert(std::is_trivially_copyable<T>::value, "Element must be trivially copyable.");

        m_cmdList->SetGraphicsRoot32BitConstant(
            rootParam, *reinterpret_cast<const UINT *>(std::addressof(value)), offset);
    }

    /// @brief
    ///   Set graphics root constant based on the specified offset.
    ///
    /// @tparam T
    ///   Type of the first root constant value to be set.
    /// @tparam ...Others
    ///   Type of the rest of root constant values to be set.
    ///
    /// @param rootParam
    ///   Index of the root constant parameter to be set.
    /// @param baseOffset
    ///   Base offset of the first 32-bit root constant value.
    /// @param value
    ///   The first 32-bit constant value to be set.
    /// @param ...others
    ///   The rest of 32-bit constant values to be set.
    template <typename T, typename... Others>
    auto setGraphicsConstant(std::uint32_t rootParam,
                             std::uint32_t baseOffset,
                             const T      &value,
                             const Others &...others) noexcept -> void {
        static_assert(sizeof(T) == sizeof(std::uint32_t),
                      "Elements must be actually 4 bytes in size.");
        static_assert(std::is_trivially_copyable<T>::value, "Elements must be trivially copyable.");

        this->setGraphicsConstant(rootParam, baseOffset, value);
        this->setGraphicsConstant(rootParam, baseOffset + 1, others...);
    }

    /// @brief
    ///   Copy data from system memory and set a constant buffer view at the specified root
    ///   parameter.
    ///
    /// @param rootParam
    ///   Index of the root parameter to set the constant buffer view.
    /// @param data
    ///   Start of data to be copied.
    /// @param size
    ///   Size in byte of data to be used as constant buffer.
    InkApi auto setGraphicsConstantBuffer(std::uint32_t rootParam,
                                          const void   *data,
                                          std::size_t   size) noexcept -> void;

    /// @brief
    ///   Copy data from system memory and set a constant buffer view for the specified root
    ///   descriptor table.
    ///
    /// @param rootParam
    ///   Index of the root descriptor table to set the constant buffer view.
    /// @param offset
    ///   Offset from start of the root descriptor table to set the constant buffer view.
    /// @param data
    ///   Start of data to be copied.
    /// @param size
    ///   Size in byte of data to be used as constant buffer.
    InkApi auto setGraphicsConstantBuffer(std::uint32_t rootParam,
                                          std::uint32_t offset,
                                          const void   *data,
                                          std::size_t   size) noexcept -> void;

    /// @brief
    ///   Set a vertex buffer to the specified slot.
    ///
    /// @param slot
    ///   Slot index to set the vertex buffer.
    /// @param gpuAddress
    ///   GPU address to start of the vertex buffer.
    /// @param vertexCount
    ///   Number of vertices to be set.
    /// @param stride
    ///   Stride size in byte of each vertex data element.
    auto setVertexBuffer(std::uint32_t slot,
                         std::uint64_t gpuAddress,
                         std::uint32_t vertexCount,
                         std::uint32_t stride) noexcept -> void {
        const D3D12_VERTEX_BUFFER_VIEW vbv{
            /* BufferLocation = */ gpuAddress,
            /* SizeInBytes    = */ vertexCount * stride,
            /* StrideInBytes  = */ stride,
        };

        m_cmdList->IASetVertexBuffers(slot, 1, &vbv);
    }

    /// @brief
    ///   Use a structured buffer as vertex buffer.
    ///
    /// @param slot
    ///   Slot index to set the vertex buffer.
    /// @param buffer
    ///   The structured buffer to be used as vertex buffer.
    auto setVertexBuffer(std::uint32_t slot, const StructuredBuffer &buffer) noexcept -> void {
        const D3D12_VERTEX_BUFFER_VIEW vbv{
            /* BufferLocation = */ buffer.gpuAddress(),
            /* SizeInBytes    = */ buffer.elementSize() * buffer.elementCount(),
            /* StrideInBytes  = */ buffer.elementSize(),
        };

        m_cmdList->IASetVertexBuffers(slot, 1, &vbv);
    }

    /// @brief
    ///   Use a temporary upload buffer as vertex buffer.
    /// @note
    ///   It is slow to upload vertex data to GPU per-frame. Please consider using static vertex
    ///   buffer if possible.
    ///
    /// @param slot
    ///   Slot index to set the vertex buffer.
    /// @param data
    ///   Pointer to start of the vertex data.
    /// @param vertexCount
    ///   Number of vertices in this vertex buffer.
    /// @param stride
    ///   Stride size in byte of each vertex element.
    InkApi auto setVertexBuffer(std::uint32_t slot,
                                const void   *data,
                                std::uint32_t vertexCount,
                                std::uint32_t stride) noexcept -> void;

    /// @brief
    ///   Set index buffer for current graphics pipeline.
    ///
    /// @param gpuAddress
    ///   GPU address to start of the index buffer.
    /// @param indexCount
    ///   Number of indices to be set.
    /// @param isUInt32
    ///   Specifies whether the indices are uint32 or uint16.
    auto setIndexBuffer(std::uint64_t gpuAddress, std::uint32_t indexCount, bool isUInt32) noexcept
        -> void {
        const DXGI_FORMAT format = isUInt32 ? DXGI_FORMAT_R32_UINT : DXGI_FORMAT_R16_UINT;
        const D3D12_INDEX_BUFFER_VIEW ibv{
            /* BufferLocation = */ gpuAddress,
            /* SizeInBytes    = */ indexCount * (isUInt32 ? 4U : 2U),
            /* Format         = */ format,
        };

        m_cmdList->IASetIndexBuffer(&ibv);
    }

    /// @brief
    ///   Use a structured buffer as index buffer.
    /// @note
    ///   Only uint16 and uint32 formats could be used as index buffer indices. Index type is
    ///   determined by structured buffer element size.
    ///
    /// @param buffer
    ///   The structured buffer to be used as index buffer.
    auto setIndexBuffer(const StructuredBuffer &buffer) noexcept -> void {
        const DXGI_FORMAT format =
            buffer.elementSize() == 2 ? DXGI_FORMAT_R16_UINT : DXGI_FORMAT_R32_UINT;
        const D3D12_INDEX_BUFFER_VIEW ibv{
            /* BufferLocation = */ buffer.gpuAddress(),
            /* SizeInBytes    = */ buffer.elementSize() * buffer.elementCount(),
            /* Format         = */ format,
        };

        m_cmdList->IASetIndexBuffer(&ibv);
    }

    /// @brief
    ///   Use a temporary upload buffer as index buffer.
    /// @note
    ///   It is slow to upload index data to GPU per-frame. Please consider using static index
    ///   buffer if possible.
    ///
    /// @param data
    ///   Pointer to start of the index data.
    /// @param indexCount
    ///   Number of indices in the index buffer.
    /// @param isUInt32
    ///   Specifies whether the index format is uint32 or uint16.
    InkApi auto setIndexBuffer(const void *data, std::uint32_t indexCount, bool isUInt32) noexcept
        -> void;

    /// @brief
    ///   Set pipeline state for this command buffer.
    /// @note
    ///   Root signature will not be affected by this method. Root signature must be set manually.
    ///
    /// @param pso
    ///   The pipeline state to be set.
    auto setPipelineState(const PipelineState &pso) noexcept -> void {
        m_cmdList->SetPipelineState(pso.pipelineState());
    }

    /// @brief
    ///   Set primitive topology for current graphics pipeline.
    ///
    /// @param topology
    ///   Primitive topology for current graphics pipeline.
    auto setPrimitiveTopology(D3D12_PRIMITIVE_TOPOLOGY topology) noexcept -> void {
        m_cmdList->IASetPrimitiveTopology(topology);
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

    /// @brief
    ///   Set color blend factors for current graphics pipeline.
    ///
    /// @param factor
    ///   Blend factors for each color options.
    auto setBlendFactor(Color factor) noexcept -> void {
        m_cmdList->OMSetBlendFactor(factor.m_arr);
    }

    /// @brief
    ///   Draw primitives.
    ///
    /// @param vertexCount
    ///   Number of vertices to be drawn.
    /// @param firstVertex
    ///   Index of the first vertex to be drawn.
    InkApi auto draw(std::uint32_t vertexCount, std::uint32_t firstVertex = 0) noexcept -> void;

    /// @brief
    ///   Draw primitives according to index buffer.
    ///
    /// @param indexCount
    ///   Number of indices to be used.
    /// @param firstIndex
    ///   Index of the first index to be used in index buffer.
    /// @param firstVertex
    ///   Index of the first vertex in vertex buffer to be drawn.
    InkApi auto drawIndexed(std::uint32_t indexCount,
                            std::uint32_t firstIndex,
                            std::uint32_t firstVertex = 0) noexcept -> void;

    /// @brief
    ///   Dispatch tasks to compute pipeline.
    ///
    /// @param groupX
    ///   Number of thread groups for x dimension.
    /// @param groupY
    ///   Number of thread groups for y dimension.
    /// @param groupZ
    ///   Number of thread groups for z dimension.
    InkApi auto dispatch(std::size_t groupX, std::size_t groupY, std::size_t groupZ) noexcept
        -> void;

private:
    /// @brief
    ///   D3D12 direct command list object.
    Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList4> m_cmdList;

    /// @brief
    ///   Command allocator for current command list.
    ID3D12CommandAllocator *m_allocator;

    /// @brief
    ///   Fence value that indicates when the last submittion of this command buffer will be
    ///   finished.
    std::uint64_t m_lastSubmitFenceValue;

    /// @brief
    ///   Temporary buffer allocator for this command list.
    DynamicBufferAllocator m_bufferAllocator;

    /// @brief
    ///   Current graphics root signature.
    RootSignature *m_graphicsRootSignature;

    /// @brief
    ///   Current compute root signature.
    RootSignature *m_computeRootSignature;

    /// @brief
    ///   Dynamic CBV/SRV/UAV descriptor heap.
    DynamicDescriptorHeap m_dynamicViewHeap;

    /// @brief
    ///   Dynamic sampler descriptor heap.
    DynamicDescriptorHeap m_dynamicSamplerHeap;
};

} // namespace ink

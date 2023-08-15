#pragma once

#include "../core/export.hpp"
#include "descriptor.hpp"

#include <wrl/client.h>

#include <array>
#include <bitset>
#include <vector>

namespace ink {

class RootSignature {
private:
    /// @brief
    ///   For internal usage. Create a new root signature.
    ///
    /// @param device
    ///   The D3D12 device that is used to create this root signature.
    /// @param desc
    ///   Root signature description structure that describes how to create this root signature.
    ///
    /// @throw RenderAPIException
    ///   Thrown if failed to create the root signature.
    RootSignature(ID3D12Device5 *device, const D3D12_ROOT_SIGNATURE_DESC &desc);

    friend class RenderDevice;

public:
    /// @brief
    ///   Create an empty root signature.
    InkExport RootSignature() noexcept;

    /// @brief
    ///   Copy constructor of root signature. Reference counting is used for D3D12 root signature
    ///   object.
    ///
    /// @param other
    ///   The root signature object to be copied from.
    InkExport RootSignature(const RootSignature &other) noexcept;

    /// @brief
    ///   Move constructor of root signature.
    ///
    /// @param other
    ///   The root signature object to be moved from. The moved root signature object will be
    ///   invalidated.
    ///
    /// @return
    ///   Reference to this root signature.
    InkExport RootSignature(RootSignature &&other) noexcept;

    /// @brief
    ///   Decrease reference counting of the root signature object and destroy it if reference
    ///   counting is 0.
    InkExport ~RootSignature() noexcept;

    /// @brief
    ///   Copy assignment of root signature. Reference counting is used for D3D12 root signature
    ///   object.
    ///
    /// @param other
    ///   The root signature object to be copied from.
    ///
    /// @return
    ///   Reference to this root signature.
    InkExport auto operator=(const RootSignature &other) noexcept -> RootSignature &;

    /// @brief
    ///   Move assignment of root signature.
    ///
    /// @param other
    ///   The root signature object to be moved from. The moved root signature object will be
    ///   invalidated.
    ///
    /// @return
    ///   Reference to this root signature.
    InkExport auto operator=(RootSignature &&other) noexcept -> RootSignature &;

    /// @brief
    ///   Checks if this is an empty root signature.
    ///
    /// @return
    ///   A boolean value that indicates whether this is an empty root signature.
    /// @retval true
    ///   This is an empty root signature.
    /// @retval false
    ///   This is not an empty root signature.
    [[nodiscard]] auto isEmpty() const noexcept -> bool { return m_rootSignature == nullptr; }

    /// @brief
    ///   Get number of static samplers in this root signature.
    ///
    /// @return
    ///   Number of static samplers in this root signature.
    [[nodiscard]] auto staticSamplerCount() const noexcept -> std::uint32_t {
        return m_staticSamplerCount;
    }

    /// @brief
    ///   Get total number of CBV/SRV/UAVs in descriptor tables.
    ///
    /// @return
    ///   Total number of CBV/SRV/UAVs in descriptor tables.
    [[nodiscard]] auto tableViewCount() const noexcept -> std::uint32_t {
        return m_tableViewDescriptorCount;
    }

    /// @brief
    ///   Get total number of samplers in descriptor tables.
    ///
    /// @return
    ///   Total number of samplers in descriptor tables.
    [[nodiscard]] auto tableSamplerCount() const noexcept -> std::uint32_t {
        return m_tableSamplerCount;
    }

    /// @brief
    ///   Checks if the specified root parameter is a CBV/SRV/UAV descriptor table.
    ///
    /// @param slot
    ///   Index of the root parameter to be checked.
    ///
    /// @return
    ///   A boolean value that indicates whether the specified root parameter is a CBV/SRV/UAV
    ///   descriptor table.
    /// @retval true
    ///   The specified root parameter is a CBV/SRV/UAV root descriptor table.
    /// @retval false
    ///   The index is out of range or the specified root parameter is not a CBV/SRV/UAV descriptor
    ///   table.
    [[nodiscard]] auto isViewTable(std::uint32_t slot) const noexcept -> bool {
        return (slot < 64) && m_viewTableFlags[slot];
    }

    /// @brief
    ///   Checks if the specified root parameter is a sampler descriptor table.
    ///
    /// @param slot
    ///   Index of the root parameter to be checked.
    ///
    /// @return
    ///   A boolean value that indicates whether the specified root parameter is a sampler
    ///   descriptor table.
    /// @retval true
    ///   The specified root parameter is a sampler descriptor table.
    /// @retval false
    ///   The index is out of range or the specified root parameter is not a sampler descriptor
    ///   table.
    [[nodiscard]] auto isSamplerTable(std::uint32_t slot) const noexcept -> bool {
        return (slot < 64) && m_samplerTableFlags[slot];
    }

    /// @brief
    ///   Get total number of descriptors in the specified descriptor table.
    ///
    /// @param slot
    ///   Index of the root descriptor table.
    ///
    /// @return
    ///   Total number of descriptors in the specified descriptor table if the specified root
    ///   parameter is a descriptor table. Otherwise, return 0.
    [[nodiscard]] auto tableSize(std::size_t slot) const noexcept -> std::uint32_t {
        return (slot < 64) ? m_tableSizes[slot] : 0;
    }

    /// @brief
    ///   Get D3D12 root signature native handle.
    ///
    /// @return
    ///   D3D12 root signature native handle. @p nullptr will be returned if this is an empty root
    ///   signature.
    [[nodiscard]] auto rootSignature() const noexcept -> ID3D12RootSignature * {
        return m_rootSignature.Get();
    }

private:
    /// @brief  D3D12 root signature object.
    Microsoft::WRL::ComPtr<ID3D12RootSignature> m_rootSignature;

    /// @brief  Total number of static samplers in this root signature.
    std::uint32_t m_staticSamplerCount;

    /// @brief  Total number of CBV/SRV/UAV descriptors in descriptor tables.
    std::uint32_t m_tableViewDescriptorCount;

    /// @brief  Total number of non-static samplers in descriptor tables.
    std::uint32_t m_tableSamplerCount;

    /// @brief  One bit is set for CBV/SRV/UAV descriptor table.
    std::bitset<64> m_viewTableFlags;

    /// @brief  One bit is set for sampler descriptor table.
    std::bitset<64> m_samplerTableFlags;

    /// @brief  Number of descriptors in each descriptor table.
    std::array<std::uint16_t, 64> m_tableSizes;
};

class PipelineState {
public:
    /// @brief
    ///   Create an empty pipeline state object.
    PipelineState() noexcept = default;

    /// @brief
    ///   Copy constructor of pipeline state object. Reference counting is used for D3D12 pipeline
    ///   state object.
    ///
    /// @param other
    ///   The pipeline state object to be copied from.
    PipelineState(const PipelineState &other) noexcept = default;

    /// @brief
    ///   Move constructor of pipeline state object.
    ///
    /// @param other
    ///   The pipeline state object to be moved from. The moved pipeline state object will be
    ///   invalidated.
    PipelineState(PipelineState &&other) noexcept = default;

    /// @brief
    ///   Destroy this pipeline state object.
    InkExport virtual ~PipelineState() noexcept;

    /// @brief
    ///   Copy assignment of pipeline state object. Reference counting is used for D3D12 pipeline
    ///   state object.
    ///
    /// @param other
    ///   The pipeline state object to be copied from.
    ///
    /// @return
    ///   Reference to this pipeline state object.
    auto operator=(const PipelineState &other) noexcept -> PipelineState & = default;

    /// @brief
    ///   Move assignment of pipeline state object.
    ///
    /// @param other
    ///   The pipeline state object to be moved from. The moved pipeline state object will be
    ///   invalidated.
    ///
    /// @return
    ///   Reference to this pipeline state object.
    auto operator=(PipelineState &&other) noexcept -> PipelineState & = default;

    /// @brief
    ///   Get D3D12 pipeline state object native handle.
    ///
    /// @return
    ///   D3D12 pipeline state object native handle.
    [[nodiscard]] auto pipelineState() const noexcept -> ID3D12PipelineState * {
        return m_pipelineState.Get();
    }

protected:
    /// @brief
    ///   D3D12 pipeline state object.
    Microsoft::WRL::ComPtr<ID3D12PipelineState> m_pipelineState;
};

class GraphicsPipelineState : public PipelineState {
protected:
    /// @brief
    ///   For internal usage. Create a new graphics pipeline state.
    ///
    /// @param device
    ///   The D3D12 device that is used to create this pipeline state.
    /// @param desc
    ///   D3D12 graphics pipeline description structure that describes how to create this graphics
    ///   pipeline state.
    ///
    /// @throw RenderAPIException
    ///   Thrown if failed to create the graphics pipeline state.
    GraphicsPipelineState(ID3D12Device *device, const D3D12_GRAPHICS_PIPELINE_STATE_DESC &desc);

    friend class RenderDevice;

public:
    /// @brief
    ///   Create an empty graphics pipeline state.
    InkExport GraphicsPipelineState() noexcept;

    /// @brief
    ///   Copy constructor of graphics pipeline state.
    ///
    /// @param other
    ///   The graphics pipeline state to be copied from.
    InkExport GraphicsPipelineState(const GraphicsPipelineState &other) noexcept;

    /// @brief
    ///   Move constructor of graphics pipeline state.
    ///
    /// @param other
    ///   The graphics pipeline state to be moved from. The moved graphics pipeline state will be
    ///   invalidated.
    InkExport GraphicsPipelineState(GraphicsPipelineState &&other) noexcept;

    /// @brief
    ///   Destroy this graphics pipeline state.
    InkExport ~GraphicsPipelineState() noexcept override;

    /// @brief
    ///   Copy assignment of graphics pipeline state.
    ///
    /// @param other
    ///   The graphics pipeline state to be copied from.
    InkExport auto operator=(const GraphicsPipelineState &other) noexcept
        -> GraphicsPipelineState &;

    /// @brief
    ///   Move assignment of graphics pipeline state.
    ///
    /// @param other
    ///   The graphics pipeline state to be moved from. The moved graphics pipeline state will be
    ///   invalidated.
    InkExport auto operator=(GraphicsPipelineState &&other) noexcept -> GraphicsPipelineState &;

    /// @brief
    ///   Get number of render targets in this graphics pipeline.
    ///
    /// @return
    ///   Number of render targets in this graphics pipeline.
    [[nodiscard]] auto renderTargetCount() const noexcept -> std::uint32_t {
        return m_renderTargetCount;
    }

    /// @brief
    ///   Get pixel format of the specified render target.
    ///
    /// @param index
    ///   Index of the render target to get pixel format for.
    ///
    /// @return
    ///   Pixel format of the specified render target. Return @p DXGI_FORMAT_UNKNOWN if index is out
    ///   of range.
    [[nodiscard]] auto renderTargetFormat(std::size_t index) const noexcept -> DXGI_FORMAT {
        return (index < m_renderTargetCount) ? m_renderTargetFormats[index] : DXGI_FORMAT_UNKNOWN;
    }

    /// @brief
    ///   Get depth stencil format of this graphics pipeline.
    ///
    /// @return
    ///   Depth stencil format of this graphics pipeline.
    [[nodiscard]] auto depthStencilFormat() const noexcept -> DXGI_FORMAT {
        return m_depthStencilFormat;
    }

    /// @brief
    ///   Get primitive type of this graphics pipeline.
    ///
    /// @return
    ///   Primitive type of this graphics pipeline.
    [[nodiscard]] auto primitiveType() const noexcept -> D3D12_PRIMITIVE_TOPOLOGY_TYPE {
        return m_primitiveType;
    }

    /// @brief
    ///   Get sample count of this graphics pipeline.
    ///
    /// @return
    ///   Sample count of this graphics pipeline.
    [[nodiscard]] auto sampleCount() const noexcept -> std::uint32_t { return m_sampleCount; }

protected:
    /// @brief
    ///   Number of render targets in this graphics pipeline.
    std::uint32_t m_renderTargetCount;

    /// @brief
    ///   Pixel formats for each render target of this graphics pipeline.
    std::array<DXGI_FORMAT, 8> m_renderTargetFormats;

    /// @brief
    ///   Depth stencil format of this graphics pipeline state.
    DXGI_FORMAT m_depthStencilFormat;

    /// @brief
    ///   Primitive type of this graphics pipeline.
    D3D12_PRIMITIVE_TOPOLOGY_TYPE m_primitiveType;

    /// @brief
    ///   Sample count of this graphics pipeline state.
    std::uint32_t m_sampleCount;
};

class ComputePipelineState : public PipelineState {
protected:
    /// @brief
    ///   For internal usage. Create a new compute pipeline state.
    ///
    /// @param device
    ///   The D3D12 device that is used to create this compute pipeline state.
    /// @param desc
    ///   D3D12 compute pipeline description structure that describes how to create this compute
    ///   pipeline state.
    ComputePipelineState(ID3D12Device *device, const D3D12_COMPUTE_PIPELINE_STATE_DESC &desc);

    friend class RenderDevice;

public:
    /// @brief
    ///   Create an empty compute pipeline state.
    InkExport ComputePipelineState() noexcept;

    /// @brief
    ///   Copy constructor of compute pipeline state.
    ///
    /// @param other
    ///   The compute pipeline state to be copied from.
    InkExport ComputePipelineState(const ComputePipelineState &other) noexcept;

    /// @brief
    ///   Move constructor of compute pipeline state.
    ///
    /// @param other
    ///   The compute pipeline state to be moved from. The moved compute pipeline state will be
    ///   invalidated.
    InkExport ComputePipelineState(ComputePipelineState &&other) noexcept;

    /// @brief
    ///   Destroy this compute pipeline state.
    InkExport ~ComputePipelineState() noexcept override;

    /// @brief
    ///   Copy assignment of compute pipeline state.
    ///
    /// @param other
    ///   The compute pipeline state to be copied from.
    InkExport auto operator=(const ComputePipelineState &other) noexcept -> ComputePipelineState &;

    /// @brief
    ///   Move assignment of compute pipeline state.
    ///
    /// @param other
    ///   The compute pipeline state to be moved from. The moved compute pipeline state will be
    ///   invalidated.
    InkExport auto operator=(ComputePipelineState &&other) noexcept -> ComputePipelineState &;
};

class DynamicDescriptorHeap {
private:
    enum class CacheType {
        None               = 0,
        CpuDescriptor      = 1,
        ConstantBufferView = 2,
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

    /// @brief
    ///   For internal usage. Create a new dynamic descriptor heap.
    ///
    /// @param renderDevice
    ///   The render device that is used to manage dynamic descriptor heaps.
    /// @param device
    ///   D3D12 device that is used to copy descriptors.
    DynamicDescriptorHeap(RenderDevice &renderDevice, ID3D12Device5 *device) noexcept;

    friend class CommandBuffer;

public:
    /// @brief
    ///   Create an empty dynamic descriptor heap.
    InkExport DynamicDescriptorHeap() noexcept;

    /// @brief
    ///   Copy constructor of dynamic descriptor heap is disabled.
    DynamicDescriptorHeap(const DynamicDescriptorHeap &) = delete;

    /// @brief
    ///   Move constructor of dynamic descriptor heap.
    ///
    /// @param other
    ///   The dynamic descriptor heap to be moved. The moved dynamic descriptor heap will be
    ///   invalidated.
    InkExport DynamicDescriptorHeap(DynamicDescriptorHeap &&other) noexcept;

    /// @brief
    ///   Destroy this dynamic descriptor heap and release all descriptors.
    InkExport ~DynamicDescriptorHeap() noexcept;

    /// @brief
    ///   Copy assignment of dynamic descriptor heap is disabled.
    auto operator=(const DynamicDescriptorHeap &) = delete;

    /// @brief
    ///   Move assignment of dynamic descriptor heap.
    ///
    /// @param other
    ///   The dynamic descriptor heap to be moved. The moved dynamic descriptor heap will be
    ///   invalidated.
    InkExport auto operator=(DynamicDescriptorHeap &&other) noexcept -> DynamicDescriptorHeap &;

    /// @brief
    ///   Parse the specified graphics root signature and prepare shader-visible descriptors for it.
    ///
    /// @param rootSig
    ///   The root signature to be parsed.
    InkExport auto parseGraphicsRootSignature(const RootSignature &rootSig) noexcept -> void;

    /// @brief
    ///   Parse the specified compute root signature and prepare shader-visible descriptors for it.
    ///
    /// @param rootSig
    ///   The root signature to be parsed.
    InkExport auto parseComputeRootSignature(const RootSignature &rootSig) noexcept -> void;

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
    InkExport auto bindGraphicsDescriptor(std::uint32_t paramIndex,
                                          std::uint32_t offset,
                                          CpuDescriptor descriptor) noexcept -> void;

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
    InkExport auto bindGraphicsDescriptor(std::uint32_t                          paramIndex,
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
    InkExport auto bindComputeDescriptor(std::uint32_t paramIndex,
                                         std::uint32_t offset,
                                         CpuDescriptor descriptor) noexcept -> void;

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
    InkExport auto bindComputeDescriptor(std::uint32_t                          paramIndex,
                                         std::uint32_t                          offset,
                                         const D3D12_CONSTANT_BUFFER_VIEW_DESC &desc) noexcept
        -> void;

    /// @brief
    ///   Upload all descriptors in the cached descriptor tables to shader-visible descriptor heaps
    ///   and bind the specified descriptor heap to current command list.
    ///
    /// @param cmdList
    ///   The command list that is used to bind the specified descriptor heap.
    ///
    /// @throw RenderAPIException
    ///   Thrown if failed to acquire new descriptor heaps.
    InkExport auto submitGraphicsDescriptors(ID3D12GraphicsCommandList *cmdList) -> void;

    /// @brief
    ///   Upload all descriptors in the cached descriptor tables to shader-visible descriptor heaps
    ///   and bind the specified descriptor heap to current command list.
    ///
    /// @param cmdList
    ///   The command list that is used to bind the specified descriptor heap.
    ///
    /// @throw RenderAPIException
    ///   Thrown if failed to acquire new descriptor heaps.
    InkExport auto submitComputeDescriptors(ID3D12GraphicsCommandList *cmdList) -> void;

    /// @brief
    ///   Clean up all descriptors and root signatures.
    ///
    /// @param fenceValue
    ///   The fence value that indicates when the allocated descriptor heaps could be reused.
    InkExport auto reset(std::uint64_t fenceValue) noexcept -> void;

private:
    /// @brief
    ///   The render device that is used to create this dynamic descriptor heap.
    RenderDevice *m_renderDevice;

    /// @brief
    ///   D3D12 device that is used to copy descriptors.
    ID3D12Device5 *m_device;

    /// @brief
    ///   CBV/SRV/UAV descriptor increment size.
    std::uint32_t m_viewSize;

    /// @brief
    ///   Sampler descriptor increment size.
    std::uint32_t m_samplerSize;

    /// @brief
    ///   Current graphics root signature.
    const RootSignature *m_graphicsRootSignature;

    /// @brief
    ///   Current compute root signature.
    const RootSignature *m_computeRootSignature;

    /// @brief
    ///   Current CBV/SRV/UAV shader-visible descriptor heap.
    ID3D12DescriptorHeap *m_viewHeap;

    /// @brief
    ///   Current sampler shader-visible descriptor heap.
    ID3D12DescriptorHeap *m_samplerHeap;

    /// @brief
    ///   Current CBV/SRV/UAV descriptor handle.
    Descriptor m_currentViewHandle;

    /// @brief
    ///   Current sampler descriptor handle.
    Descriptor m_currentSamplerHandle;

    /// @brief
    ///   Number of free CBV/SRV/UAV descriptors in current shader-visible descriptor heap.
    std::uint32_t m_freeViewCount;

    /// @brief
    ///   Number of free sampler descriptors in current shader-visible descriptor heap.
    std::uint32_t m_freeSamplerCount;

    /// @brief
    ///   Retired CBV/SRV/UAV descriptor heaps.
    std::vector<ID3D12DescriptorHeap *> m_retiredViewHeaps;

    /// @brief
    ///   Retired sampler descriptor heaps.
    std::vector<ID3D12DescriptorHeap *> m_retiredSamplerHeaps;

    /// @brief
    ///   Cached graphics descriptors.
    std::vector<DescriptorCache> m_graphicsDescriptors;

    /// @brief
    ///   Cached compute descriptors.
    std::vector<DescriptorCache> m_computeDescriptors;

    /// @brief
    ///   Cached descriptor table ranges for graphics root signature.
    std::array<DescriptorTableRange, 64> m_graphicsTableRange;

    /// @brief
    ///   Cached descriptor table ranges for compute root signature.
    std::array<DescriptorTableRange, 64> m_computeTableRange;
};

} // namespace ink

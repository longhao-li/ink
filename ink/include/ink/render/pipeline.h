#pragma once

#include "ink/core/common.h"

#include <d3d12.h>
#include <wrl/client.h>

#include <bitset>

namespace ink {

class RootSignature {
public:
    /// @brief
    ///   Create an empty root signature.
    InkApi RootSignature() noexcept;

    /// @brief
    ///   Create a new D3D12 root signature.
    /// @note
    ///   Errors are handled with assertions.
    ///
    /// @param desc
    ///   The root signature description structure that describes how to create the new root
    ///   signature.
    InkApi RootSignature(const D3D12_ROOT_SIGNATURE_DESC &desc) noexcept;

    /// @brief
    ///   Copy constructor of root signature. Reference counting is used for D3D12 root signature
    ///   object.
    ///
    /// @param other
    ///   The root signature object to be copied from.
    InkApi RootSignature(const RootSignature &other) noexcept;

    /// @brief
    ///   Copy assignment of root signature. Reference counting is used for D3D12 root signature
    ///   object.
    ///
    /// @param other
    ///   The root signature object to be copied from.
    ///
    /// @return
    ///   Reference to this root signature.
    InkApi auto operator=(const RootSignature &other) noexcept -> RootSignature &;

    /// @brief
    ///   Move constructor of root signature.
    ///
    /// @param other
    ///   The root signature object to be moved from. The moved root signature object will be
    ///   invalidated.
    ///
    /// @return
    ///   Reference to this root signature.
    InkApi RootSignature(RootSignature &&other) noexcept;

    /// @brief
    ///   Move assignment of root signature.
    ///
    /// @param other
    ///   The root signature object to be moved from. The moved root signature object will be
    ///   invalidated.
    ///
    /// @return
    ///   Reference to this root signature.
    InkApi auto operator=(RootSignature &&other) noexcept -> RootSignature &;

    /// @brief
    ///   Decrease reference counting of the root signature object and destroy it if reference
    ///   counting is 0.
    InkApi ~RootSignature() noexcept;

    /// @brief
    ///   Checks if this is an empty root signature.
    ///
    /// @return
    ///   A boolean value that indicates whether this is an empty root signature.
    /// @retval true
    ///   This is an empty root signature.
    /// @retval false
    ///   This is not an empty root signature.
    [[nodiscard]]
    auto isEmpty() const noexcept -> bool {
        return m_rootSignature == nullptr;
    }

    /// @brief
    ///   Get number of static samplers in this root signature.
    ///
    /// @return
    ///   Number of static samplers in this root signature.
    [[nodiscard]]
    auto staticSamplerCount() const noexcept -> std::uint32_t {
        return m_staticSamplerCount;
    }

    /// @brief
    ///   Get total number of CBV/SRV/UAVs in descriptor tables.
    ///
    /// @return
    ///   Total number of CBV/SRV/UAVs in descriptor tables.
    [[nodiscard]]
    auto tableViewCount() const noexcept -> std::uint32_t {
        return m_tableViewDescriptorCount;
    }

    /// @brief
    ///   Get total number of samplers in descriptor tables.
    ///
    /// @return
    ///   Total number of samplers in descriptor tables.
    [[nodiscard]]
    auto tableSamplerCount() const noexcept -> std::uint32_t {
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
    [[nodiscard]]
    auto isViewTable(std::uint32_t slot) const noexcept -> bool {
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
    [[nodiscard]]
    auto isSamplerTable(std::uint32_t slot) const noexcept -> bool {
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
    [[nodiscard]]
    auto tableSize(std::size_t slot) const noexcept -> std::uint32_t {
        return (slot < 64) ? m_tableSizes[slot] : 0;
    }

    /// @brief
    ///   Get D3D12 root signature native handle.
    ///
    /// @return
    ///   D3D12 root signature native handle. @p nullptr will be returned if this is an empty root
    ///   signature.
    [[nodiscard]]
    auto rootSignature() const noexcept -> ID3D12RootSignature * {
        return m_rootSignature.Get();
    }

private:
    /// @brief
    ///   D3D12 root signature object.
    Microsoft::WRL::ComPtr<ID3D12RootSignature> m_rootSignature;

    /// @brief
    ///   Total number of static samplers in this root signature.
    std::uint32_t m_staticSamplerCount;

    /// @brief
    ///   Total number of CBV/SRV/UAV descriptors in descriptor tables.
    std::uint32_t m_tableViewDescriptorCount;

    /// @brief
    ///   Total number of non-static samplers in descriptor tables.
    std::uint32_t m_tableSamplerCount;

    /// @brief
    ///   One bit is set for CBV/SRV/UAV descriptor table.
    std::bitset<64> m_viewTableFlags;

    /// @brief
    ///   One bit is set for sampler descriptor table.
    std::bitset<64> m_samplerTableFlags;

    /// @brief
    ///   Number of descriptors in each descriptor table.
    std::uint16_t m_tableSizes[64];
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
    ///   Move constructor of pipeline state object.
    ///
    /// @param other
    ///   The pipeline state object to be moved from. The moved pipeline state object will be
    ///   invalidated.
    PipelineState(PipelineState &&other) noexcept = default;

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
    ///   Destroy this pipeline state object.
    InkApi virtual ~PipelineState() noexcept;

    /// @brief
    ///   Get D3D12 pipeline state object native handle.
    ///
    /// @return
    ///   D3D12 pipeline state object native handle.
    [[nodiscard]]
    auto pipelineState() const noexcept -> ID3D12PipelineState * {
        return m_pipelineState.Get();
    }

protected:
    /// @brief
    ///   D3D12 pipeline state object.
    Microsoft::WRL::ComPtr<ID3D12PipelineState> m_pipelineState;
};

class GraphicsPipelineState : public PipelineState {
public:
    /// @brief
    ///   Create an empty graphics pipeline state object.
    InkApi GraphicsPipelineState() noexcept;

    /// @brief
    ///   Create a new graphics pipeline state object.
    /// @note
    ///   Errors are handled with assertions.
    ///
    /// @param desc
    ///   A D3D12 graphics pipeline state description structure that describes how to create this
    ///   graphics pipeline state object.
    InkApi GraphicsPipelineState(const D3D12_GRAPHICS_PIPELINE_STATE_DESC &desc) noexcept;

    /// @brief
    ///   Copy constructor of graphics pipeline state. Reference counting is used for D3D12 pipeline
    ///   state object.
    ///
    /// @param other
    ///   The graphics pipeline state object to be copied from.
    InkApi GraphicsPipelineState(const GraphicsPipelineState &other) noexcept;

    /// @brief
    ///   Move constructor of graphics pipeline state.
    ///
    /// @param other
    ///   The graphics pipeline state object to be moved from. The moved graphics pipeline state
    ///   object will be invalidated.
    InkApi GraphicsPipelineState(GraphicsPipelineState &&other) noexcept;

    /// @brief
    ///   Move assignment of graphics pipeline state.
    ///
    /// @param other
    ///   The graphics pipeline state object to be moved from. The moved graphics pipeline state
    ///   object will be invalidated.
    ///
    /// @return
    ///   Reference to this graphics pipeline state.
    InkApi auto operator=(GraphicsPipelineState &&other) noexcept -> GraphicsPipelineState &;

    /// @brief
    ///   Decrease reference counting of current graphics pipeline state and destroy it if the
    ///   reference counting is 0.
    InkApi ~GraphicsPipelineState() noexcept override;

    /// @brief
    ///   Get number of render targets in this graphics pipeline.
    ///
    /// @return
    ///   Number of render targets in this graphics pipeline.
    [[nodiscard]]
    auto renderTargetCount() const noexcept -> std::uint32_t {
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
    [[nodiscard]]
    auto renderTargetFormat(std::size_t index) const noexcept -> DXGI_FORMAT {
        return (index < m_renderTargetCount) ? m_renderTargetFormats[index] : DXGI_FORMAT_UNKNOWN;
    }

    /// @brief
    ///   Get depth stencil format of this graphics pipeline.
    ///
    /// @return
    ///   Depth stencil format of this graphics pipeline.
    [[nodiscard]]
    auto depthStencilFormat() const noexcept -> DXGI_FORMAT {
        return m_depthStencilFormat;
    }

    /// @brief
    ///   Get primitive type of this graphics pipeline.
    ///
    /// @return
    ///   Primitive type of this graphics pipeline.
    [[nodiscard]]
    auto primitiveType() const noexcept -> D3D12_PRIMITIVE_TOPOLOGY_TYPE {
        return m_primitiveType;
    }

    /// @brief
    ///   Get sample count of this graphics pipeline.
    ///
    /// @return
    ///   Sample count of this graphics pipeline.
    [[nodiscard]]
    auto sampleCount() const noexcept -> std::uint32_t {
        return m_sampleCount;
    }

protected:
    /// @brief
    ///   Number of render targets in this graphics pipeline.
    std::uint32_t m_renderTargetCount;

    /// @brief
    ///   Pixel formats for each render target of this graphics pipeline.
    DXGI_FORMAT m_renderTargetFormats[8];

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

} // namespace ink

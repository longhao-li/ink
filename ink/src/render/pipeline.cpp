#include "ink/render/pipeline.hpp"
#include "ink/core/exception.hpp"

using namespace ink;
using Microsoft::WRL::ComPtr;

ink::RootSignature::RootSignature(ID3D12Device5 *device, const D3D12_ROOT_SIGNATURE_DESC &desc)
    : m_rootSignature(),
      m_staticSamplerCount(desc.NumStaticSamplers),
      m_tableViewDescriptorCount(),
      m_tableSamplerCount(),
      m_viewTableFlags(),
      m_samplerTableFlags(),
      m_tableSizes() {
    // Serialize the root signature.
    ComPtr<ID3DBlob> signature;
    ComPtr<ID3DBlob> error;

    HRESULT hr = D3D12SerializeRootSignature(&desc, D3D_ROOT_SIGNATURE_VERSION_1_0,
                                             signature.GetAddressOf(), error.GetAddressOf());
    if (FAILED(hr)) {
        std::string msg("Failed to serialize root signature: ");
        msg += static_cast<char *>(error->GetBufferPointer());
        throw RenderAPIException(hr, std::move(msg));
    }

    // Create the root signature.
    hr = device->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(),
                                     IID_PPV_ARGS(m_rootSignature.GetAddressOf()));
    if (FAILED(hr))
        throw RenderAPIException(hr, "Failed to create D3D12 root signature.");

    // Cache root signature metadata.
    for (std::uint32_t i = 0; i < desc.NumParameters; ++i) {
        const auto &param = desc.pParameters[i];
        if (param.ParameterType == D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE) {
            const auto &table      = param.DescriptorTable;
            const auto *rangeBegin = table.pDescriptorRanges;
            const auto *rangeEnd   = rangeBegin + table.NumDescriptorRanges;

            for (auto j = rangeBegin; j != rangeEnd; ++j)
                m_tableSizes[i] += static_cast<std::uint16_t>(j->NumDescriptors);

            if (rangeBegin->RangeType == D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER) {
                m_samplerTableFlags[i] = true;
                m_tableSamplerCount += m_tableSizes[i];
            } else {
                m_viewTableFlags[i] = true;
                m_tableViewDescriptorCount += m_tableSizes[i];
            }
        }
    }
}

ink::RootSignature::RootSignature(const RootSignature &other) noexcept = default;

ink::RootSignature::RootSignature(RootSignature &&other) noexcept = default;

ink::RootSignature::~RootSignature() noexcept = default;

auto ink::RootSignature::operator=(const RootSignature &other) noexcept
    -> RootSignature & = default;

auto ink::RootSignature::operator=(RootSignature &&other) noexcept -> RootSignature & = default;

ink::PipelineState::~PipelineState() noexcept = default;

ink::GraphicsPipelineState::GraphicsPipelineState(ID3D12Device                             *device,
                                                  const D3D12_GRAPHICS_PIPELINE_STATE_DESC &desc)
    : PipelineState(),
      m_renderTargetCount(desc.NumRenderTargets),
      m_renderTargetFormats{
          desc.RTVFormats[0], desc.RTVFormats[1], desc.RTVFormats[2], desc.RTVFormats[3],
          desc.RTVFormats[4], desc.RTVFormats[5], desc.RTVFormats[6], desc.RTVFormats[7],
      },
      m_depthStencilFormat(desc.DSVFormat),
      m_primitiveType(desc.PrimitiveTopologyType),
      m_sampleCount(desc.SampleDesc.Count) {
    HRESULT hr =
        device->CreateGraphicsPipelineState(&desc, IID_PPV_ARGS(m_pipelineState.GetAddressOf()));
    if (FAILED(hr))
        throw RenderAPIException(hr, "Failed to create graphics pipeline state.");
}

ink::GraphicsPipelineState::GraphicsPipelineState() noexcept
    : PipelineState(),
      m_renderTargetCount(),
      m_renderTargetFormats(),
      m_depthStencilFormat(),
      m_primitiveType(),
      m_sampleCount() {}

ink::GraphicsPipelineState::GraphicsPipelineState(const GraphicsPipelineState &other) noexcept =
    default;

ink::GraphicsPipelineState::GraphicsPipelineState(GraphicsPipelineState &&other) noexcept = default;

ink::GraphicsPipelineState::~GraphicsPipelineState() noexcept = default;

auto ink::GraphicsPipelineState::operator=(const GraphicsPipelineState &other) noexcept
    -> GraphicsPipelineState & = default;

auto ink::GraphicsPipelineState::operator=(GraphicsPipelineState &&other) noexcept
    -> GraphicsPipelineState & = default;

ink::ComputePipelineState::ComputePipelineState(ID3D12Device                            *device,
                                                const D3D12_COMPUTE_PIPELINE_STATE_DESC &desc)
    : PipelineState() {
    HRESULT hr =
        device->CreateComputePipelineState(&desc, IID_PPV_ARGS(m_pipelineState.GetAddressOf()));
    if (FAILED(hr))
        throw RenderAPIException(hr, "Failed to create compute pipeline state.");
}

ink::ComputePipelineState::ComputePipelineState() noexcept : PipelineState() {}

ink::ComputePipelineState::ComputePipelineState(const ComputePipelineState &other) noexcept =
    default;

ink::ComputePipelineState::ComputePipelineState(ComputePipelineState &&other) noexcept = default;

ink::ComputePipelineState::~ComputePipelineState() noexcept = default;

auto ink::ComputePipelineState::operator=(const ComputePipelineState &other) noexcept
    -> ComputePipelineState & = default;

auto ink::ComputePipelineState::operator=(ComputePipelineState &&other) noexcept
    -> ComputePipelineState & = default;

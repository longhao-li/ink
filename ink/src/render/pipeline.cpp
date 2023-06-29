#include "ink/render/pipeline.h"
#include "ink/core/assert.h"
#include "ink/render/device.h"

#include <d3dcompiler.h>

using namespace ink;
using Microsoft::WRL::ComPtr;

ink::RootSignature::RootSignature() noexcept
    : m_rootSignature(),
      m_staticSamplerCount(),
      m_tableViewDescriptorCount(),
      m_tableSamplerCount(),
      m_viewTableFlags(),
      m_samplerTableFlags(),
      m_tableSizes() {}

ink::RootSignature::RootSignature(const D3D12_ROOT_SIGNATURE_DESC &desc) noexcept
    : m_rootSignature(),
      m_staticSamplerCount(desc.NumStaticSamplers),
      m_tableViewDescriptorCount(),
      m_tableSamplerCount(),
      m_viewTableFlags(),
      m_samplerTableFlags(),
      m_tableSizes() {
    [[maybe_unused]] HRESULT hr;

    // Serialize the root signature.
    ComPtr<ID3DBlob>                    binDesc;
    D3D12_VERSIONED_ROOT_SIGNATURE_DESC versionedDesc;
    versionedDesc.Version  = D3D_ROOT_SIGNATURE_VERSION_1_0;
    versionedDesc.Desc_1_0 = desc;

    hr = D3D12SerializeVersionedRootSignature(&versionedDesc, binDesc.GetAddressOf(), nullptr);
    inkAssert(SUCCEEDED(hr), u"Failed to serialize D3D12 root signature: 0x{:X}.",
              static_cast<std::uint32_t>(hr));

    // Create the root signature.
    auto &dev = RenderDevice::singleton();
    hr = dev.device()->CreateRootSignature(0, binDesc->GetBufferPointer(), binDesc->GetBufferSize(),
                                           IID_PPV_ARGS(m_rootSignature.GetAddressOf()));
    inkAssert(SUCCEEDED(hr), u"Failed to create D3D12 root signature: 0x{:X}.",
              static_cast<std::uint32_t>(hr));

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
                m_samplerTableFlags[i] = 1;
                m_tableSamplerCount += m_tableSizes[i];
            } else {
                m_viewTableFlags[i] = 1;
                m_tableViewDescriptorCount += m_tableSizes[i];
            }
        }
    }
}

ink::RootSignature::RootSignature(const RootSignature &other) noexcept = default;

auto ink::RootSignature::operator=(const RootSignature &other) noexcept
    -> RootSignature & = default;

ink::RootSignature::RootSignature(RootSignature &&other) noexcept = default;

auto ink::RootSignature::operator=(RootSignature &&other) noexcept -> RootSignature & = default;

ink::RootSignature::~RootSignature() noexcept {}
#pragma once

#include "ink/render/descriptor.h"

#include <wrl/client.h>

namespace ink {

class GpuResource {
public:
    /// @brief
    ///   Create an empty GPU resource object.
    GpuResource() noexcept : m_resource(), m_usageState(D3D12_RESOURCE_STATE_COMMON) {}

    /// @brief
    ///   Copy constructor of GPU resource is disabled.
    GpuResource(const GpuResource &) = delete;

    /// @brief
    ///   Copy assignment of GPU resource is disabled.
    auto operator=(const GpuResource &) = delete;

    /// @brief
    ///   Move constructor of GPU resource.
    ///
    /// @param other
    ///   The GPU resource object to be moved. The moved GPU resource will be invalidated.
    GpuResource(GpuResource &&other) noexcept = default;

    /// @brief
    ///   Move assignment of GPU resource.
    ///
    /// @param other
    ///   The GPU resource object to be moved. The moved GPU resource will be invalidated.
    ///
    /// @return
    ///   Reference to this GPU resource.
    auto operator=(GpuResource &&other) noexcept -> GpuResource & = default;

    /// @brief
    ///   Destroy this GPU resource.
    virtual ~GpuResource() noexcept;

    /// @brief
    ///   Get GPU usage state of this GPU resource.
    ///
    /// @return
    ///   GPU usage state of this GPU resource.
    [[nodiscard]]
    auto state() const noexcept -> D3D12_RESOURCE_STATES {
        return m_usageState;
    }

    friend class CommandBuffer;

protected:
    /// @brief
    ///   D3D12 resource object of this GPU resource.
    Microsoft::WRL::ComPtr<ID3D12Resource> m_resource;

    /// @brief
    ///   D3D12 resource state.
    D3D12_RESOURCE_STATES m_usageState;
};

class GpuBuffer : public GpuResource {
public:
    /// @brief
    ///   Create an empty GPU buffer.
    GpuBuffer() noexcept;

    /// @brief
    ///   Create a new GPU buffer with at least the specified size.
    /// @note
    ///   Errors are handled with assertion and no exception may be thrown.
    ///
    /// @param size
    ///   Expected size in byte of this GPU buffer. This value may be changed due to alignment.
    explicit GpuBuffer(std::size_t size) noexcept;

    /// @brief
    ///   Move constructor of GPU buffer.
    ///
    /// @param other
    ///   The GPU buffer to be moved. The moved GPU buffer will be invalidated.
    GpuBuffer(GpuBuffer &&other) noexcept;

    /// @brief
    ///   Move assignment of GPU buffer.
    ///
    /// @param other
    ///   The GPU buffer to be moved. The moved GPU buffer will be invalidated.
    ///
    /// @return
    ///   Reference to this GPU buffer.
    auto operator=(GpuBuffer &&other) noexcept -> GpuBuffer &;

private:
    /// @brief
    ///   Size in byte of this GPU buffer.
    std::size_t m_size;

    /// @brief
    ///   GPU virtual address to start of this GPU buffer.
    std::uint64_t m_gpuAddress;

    /// @brief
    ///   Byte address unordered access view of this GPU buffer. It is guaranteed that GPU buffers
    ///   support byte address unordered access.
    UnorderedAccessView m_byteAddressUAV;
};

} // namespace ink

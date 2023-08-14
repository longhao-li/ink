#pragma once

#include "../core/export.hpp"

#include <d3d12.h>

#include <cstddef>
#include <cstdint>
#include <limits>

namespace ink {

class CpuDescriptor {
public:
    /// @brief
    ///   Create a null CPU descriptor.
    constexpr CpuDescriptor() noexcept : m_handle(std::numeric_limits<std::size_t>::max()) {}

    /// @brief
    ///   Create a CPU descriptor from D3D12 CPU descriptor handle.
    ///
    /// @param handle
    ///   D3D12 CPU descriptor handle.
    constexpr CpuDescriptor(D3D12_CPU_DESCRIPTOR_HANDLE handle) noexcept : m_handle(handle.ptr) {}

    /// @brief
    ///   Checks if this CPU descriptor is null.
    ///
    /// @return
    ///   A boolean value that indicates whether this CPU descriptor handle is null.
    /// @retval true
    ///   This is a null CPU descriptor.
    /// @retval false
    ///   This is not a null CPU descriptor.
    [[nodiscard]] constexpr auto isNull() const noexcept -> bool {
        return m_handle == std::numeric_limits<std::size_t>::max();
    }

    /// @brief
    ///   Get the underlying D3D12 CPU descriptor handle pointer.
    ///
    /// @return
    ///   The underlying D3D12 CPU descriptor handle pointer.
    [[nodiscard]] constexpr auto value() const noexcept -> std::size_t { return m_handle; }

    /// @brief
    ///   Apply an offset to this CPU descriptor.
    ///
    /// @param offset
    ///   The offset to be applied. Could either be positive or negative.
    ///
    /// @return
    ///   Reference to this CPU descriptor.
    constexpr auto operator+=(std::ptrdiff_t offset) noexcept -> CpuDescriptor & {
        m_handle = static_cast<std::size_t>(static_cast<std::ptrdiff_t>(m_handle) + offset);
        return *this;
    }

    /// @brief
    ///   Apply an offset to this CPU descriptor.
    ///
    /// @param offset
    ///   The offset to be applied. Could either be positive or negative.
    ///
    /// @return
    ///   Reference to this CPU descriptor.
    constexpr auto operator-=(std::ptrdiff_t offset) noexcept -> CpuDescriptor & {
        m_handle = static_cast<std::size_t>(static_cast<std::ptrdiff_t>(m_handle) - offset);
        return *this;
    }

    /// @brief
    ///   Allow implicit conversion to D3D12 CPU descriptor handle.
    constexpr operator D3D12_CPU_DESCRIPTOR_HANDLE() const noexcept {
        return {static_cast<SIZE_T>(m_handle)};
    }

    constexpr auto operator==(CpuDescriptor rhs) const noexcept -> bool {
        return m_handle == rhs.m_handle;
    }

    constexpr auto operator!=(CpuDescriptor rhs) const noexcept -> bool {
        return m_handle != rhs.m_handle;
    }

    constexpr auto operator<(CpuDescriptor rhs) const noexcept -> bool {
        return m_handle < rhs.m_handle;
    }

    constexpr auto operator<=(CpuDescriptor rhs) const noexcept -> bool {
        return m_handle <= rhs.m_handle;
    }

    constexpr auto operator>(CpuDescriptor rhs) const noexcept -> bool {
        return m_handle > rhs.m_handle;
    }

    constexpr auto operator>=(CpuDescriptor rhs) const noexcept -> bool {
        return m_handle >= rhs.m_handle;
    }

private:
    /// @brief
    ///   CPU descriptor handle value.
    std::size_t m_handle;
};

constexpr auto operator+(CpuDescriptor lhs, std::ptrdiff_t rhs) noexcept -> CpuDescriptor {
    lhs += rhs;
    return lhs;
}

constexpr auto operator+(std::ptrdiff_t lhs, CpuDescriptor rhs) noexcept -> CpuDescriptor {
    rhs += lhs;
    return rhs;
}

constexpr auto operator-(CpuDescriptor lhs, std::ptrdiff_t rhs) noexcept -> CpuDescriptor {
    lhs -= rhs;
    return lhs;
}

class Descriptor {
public:
    /// @brief
    ///   Create a null shader visible descriptor.
    constexpr Descriptor() noexcept
        : m_cpuHandle(std::numeric_limits<std::size_t>::max()),
          m_gpuHandle(std::numeric_limits<std::uint64_t>::max()) {}

    /// @brief
    ///   Create a shader-visible descriptor from D3D12 CPU and GPU handle.
    ///
    /// @param cpuHandle
    ///   The CPU descriptor handle that this descriptor handle is created from.
    /// @param gpuHandle
    ///   The GPU descriptor handle that this descriptor handle is created from.
    constexpr Descriptor(D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle,
                         D3D12_GPU_DESCRIPTOR_HANDLE gpuHandle) noexcept
        : m_cpuHandle(cpuHandle.ptr), m_gpuHandle(gpuHandle.ptr) {}

    /// @brief
    ///   Checks if this is a null shader-visible descriptor handle.
    ///
    /// @return bool
    ///   A boolean value that indicates whether this is a null descriptor handle.
    /// @retval true
    ///   This is a null shader-visible descriptor handle.
    /// @retval false
    ///   This is not a null shader-visible descriptor handle.
    [[nodiscard]] constexpr auto isNull() const noexcept -> bool {
        return m_cpuHandle == std::numeric_limits<std::size_t>::max();
    }

    /// @brief
    ///   Apply an offset to this shader visible descriptor.
    ///
    /// @param offset
    ///   The offset to be applied. Could either be positive or negative.
    ///
    /// @return
    ///   Reference to this shader visible descriptor.
    constexpr auto operator+=(std::ptrdiff_t offset) noexcept -> Descriptor & {
        m_cpuHandle = static_cast<std::size_t>(static_cast<std::ptrdiff_t>(m_cpuHandle) + offset);
        m_gpuHandle = static_cast<std::uint64_t>(static_cast<std::int64_t>(m_gpuHandle) + offset);
        return *this;
    }

    /// @brief
    ///   Apply an offset to this shader visible descriptor.
    ///
    /// @param offset
    ///   The offset to be applied. Could either be positive or negative.
    ///
    /// @return
    ///   Reference to this shader visible descriptor.
    constexpr auto operator-=(std::ptrdiff_t offset) noexcept -> Descriptor & {
        m_cpuHandle = static_cast<std::size_t>(static_cast<std::ptrdiff_t>(m_cpuHandle) - offset);
        m_gpuHandle = static_cast<std::uint64_t>(static_cast<std::int64_t>(m_gpuHandle) - offset);
        return *this;
    }

    constexpr auto operator==(Descriptor rhs) const noexcept -> bool {
        return m_cpuHandle == rhs.m_cpuHandle && m_gpuHandle == rhs.m_gpuHandle;
    }

    constexpr auto operator!=(Descriptor rhs) const noexcept -> bool {
        return m_cpuHandle != rhs.m_cpuHandle || m_gpuHandle != rhs.m_gpuHandle;
    }

    /// @brief
    ///   Allow implicit conversion to D3D12 CPU descriptor handle.
    constexpr operator D3D12_CPU_DESCRIPTOR_HANDLE() const noexcept {
        return {static_cast<SIZE_T>(m_cpuHandle)};
    }

    /// @brief
    ///   Allow implicit conversion to D3D12 GPU descriptor handle.
    constexpr operator D3D12_GPU_DESCRIPTOR_HANDLE() const noexcept {
        return {static_cast<UINT64>(m_gpuHandle)};
    }

private:
    /// @brief
    ///   CPU descriptor handle for this shader visible descriptor.
    std::size_t m_cpuHandle;

    /// @brief
    ///   GPU descriptor handle for this shader visible descriptor.
    std::uint64_t m_gpuHandle;
};

constexpr auto operator+(Descriptor lhs, std::ptrdiff_t rhs) noexcept -> Descriptor {
    lhs += rhs;
    return lhs;
}

constexpr auto operator+(std::ptrdiff_t lhs, Descriptor rhs) noexcept -> Descriptor {
    rhs += lhs;
    return rhs;
}

constexpr auto operator-(Descriptor lhs, std::ptrdiff_t rhs) noexcept -> Descriptor {
    lhs -= rhs;
    return lhs;
}

class RenderDevice;

class ConstantBufferView {
private:
    /// @brief
    ///   For internal usage. Create a new constant buffer view.
    ///
    /// @param renderDevice
    ///   The render device that creates this constant buffer view.
    /// @param descriptor
    ///   The CPU descriptor handle for this constant buffer view.
    ConstantBufferView(RenderDevice &renderDevice, CpuDescriptor descriptor) noexcept
        : m_renderDevice(&renderDevice), m_handle(descriptor) {}

    friend class RenderDevice;

public:
    /// @brief
    ///   Create a null constant buffer view.
    InkExport ConstantBufferView() noexcept;

    /// @brief
    ///   Copy constructor of constant buffer view is disabled.
    ConstantBufferView(const ConstantBufferView &) = delete;

    /// @brief
    ///   Move constructor of constant buffer view.
    ///
    /// @param other
    ///   The constant buffer view to be moved. The moved constant buffer view will be set to null.
    InkExport ConstantBufferView(ConstantBufferView &&other) noexcept;

    /// @brief
    ///   Destroy this constant buffer view and release the descriptor.
    InkExport ~ConstantBufferView() noexcept;

    /// @brief
    ///   Copy assignment of constant buffer view is disabled.
    auto operator=(const ConstantBufferView &) = delete;

    /// @brief
    ///   Move assignment of constant buffer view.
    ///
    /// @param other
    ///   The constant buffer view to be moved. The moved constant buffer view will be set to null.
    ///
    /// @return
    ///   Reference to this constant buffer view.
    InkExport auto operator=(ConstantBufferView &&other) noexcept -> ConstantBufferView &;

    /// @brief
    ///   Update this constant buffer view.
    /// @note
    ///   Null constant buffer view cannot be updated.
    ///
    /// @param gpuAddress
    ///   The GPU address of the constant buffer.
    /// @param size
    ///   Size in byte of the constant buffer view.
    InkExport auto update(std::uint64_t gpuAddress, std::uint32_t size) noexcept -> void;

    /// @brief
    ///   Update this constant buffer view as a shader resource view.
    /// @note
    ///   Null shader resource view cannot be updated.
    ///
    /// @param resource
    ///   The resource that this shader resource view is created from.
    /// @param desc
    ///   A description structure that describes how to create this shader resource view. @p
    ///   resource must be non-null if @p desc is null.
    InkExport auto update(ID3D12Resource                        *resource,
                          const D3D12_SHADER_RESOURCE_VIEW_DESC *desc) noexcept -> void;

    /// @brief
    ///   Update this contant buffer view as an unordered access view.
    /// @note
    ///   Null unordered access view cannot be updated.
    ///
    /// @param resource
    ///   The resource that this unordered access view is created from.
    /// @param counter
    ///   The counter resource for this unordered access view.
    /// @param desc
    ///   A description structure that describes how to create this unordered access view. @p
    ///   resource must be non-null if @p desc is null.
    InkExport auto update(ID3D12Resource                         *resource,
                          ID3D12Resource                         *counter,
                          const D3D12_UNORDERED_ACCESS_VIEW_DESC *desc) noexcept -> void;

    /// @brief
    ///   Checks if this is a null constant buffer view.
    ///
    /// @return
    ///   A boolean value that indicates whether this is a null constant buffer view.
    /// @retval true
    ///   This is a null constant buffer view.
    /// @retval false
    ///   This is not a null constant buffer view.
    [[nodiscard]] auto isNull() const noexcept -> bool { return m_handle.isNull(); }

    /// @brief
    ///   Get CPU descriptor handle of this constant buffer view.
    ///
    /// @return
    ///   CPU descriptor handle of this constant buffer view.
    [[nodiscard]] auto descriptor() const noexcept -> CpuDescriptor { return m_handle; }

    friend class RenderDevice;

private:
    /// @brief
    ///   The render device that this constant buffer view is allocated from.
    RenderDevice *m_renderDevice;

    /// @brief
    ///   CPU descriptor handle of this constant buffer view.
    CpuDescriptor m_handle;
};

/// @brief
///   Constant buffer view descriptors could be used as shader resource views in D3D12.
using ShaderResourceView = ConstantBufferView;

/// @brief
///   Constant buffer view descriptors could be used as unordered access views in D3D12.
using UnorderedAccessView = ConstantBufferView;

class RenderTargetView {
private:
    /// @brief
    ///   For internal usage. Create a new render target view.
    ///
    /// @param renderDevice
    ///   The render device that creates this render target view.
    /// @param descriptor
    ///   The CPU descriptor handle for this render target view.
    RenderTargetView(RenderDevice &renderDevice, CpuDescriptor descriptor) noexcept
        : m_renderDevice(&renderDevice), m_handle(descriptor) {}

    friend class RenderDevice;

public:
    /// @brief
    ///   Create a null render target view.
    InkExport RenderTargetView() noexcept;

    /// @brief
    ///   Copy constructor of render target view is disabled.
    RenderTargetView(const RenderTargetView &) = delete;

    /// @brief
    ///   Move constructor of render target view.
    ///
    /// @param other
    ///   The render target view to be moved. The moved render target view will be set to null.
    InkExport RenderTargetView(RenderTargetView &&other) noexcept;

    /// @brief
    ///   Destroy this render target view and release the CPU descriptor.
    InkExport ~RenderTargetView() noexcept;

    /// @brief
    ///   Copy assignment of render target view is disabled.
    auto operator=(const RenderTargetView &) = delete;

    /// @brief
    ///   Move assignment of render target view.
    ///
    /// @param other
    ///   The render target view to be moved. The moved render target view will be set to null.
    ///
    /// @return
    ///   Reference to this render target view.
    InkExport auto operator=(RenderTargetView &&other) noexcept -> RenderTargetView &;

    /// @brief
    ///   Update this render target view.
    /// @note
    ///   Null render target view cannot be updated.
    ///
    /// @param resource
    ///   The resource that this render target view is created from.
    /// @param desc
    ///   A description structure that describes how to create this render target view. @p resource
    ///   must be non-null if @p desc is null.
    InkExport auto update(ID3D12Resource                      *resource,
                          const D3D12_RENDER_TARGET_VIEW_DESC *desc) noexcept -> void;

    /// @brief
    ///   Checks if this is a null render target view.
    ///
    /// @return
    ///   A boolean value that indicates whether this is a null render target view.
    /// @retval true
    ///   This is a null render target view.
    /// @retval false
    ///   This is not a null render target view.
    [[nodiscard]] auto isNull() const noexcept -> bool { return m_handle.isNull(); }

    /// @brief
    ///   Get CPU descriptor handle of this render target view.
    ///
    /// @return
    ///   CPU descriptor handle of this render target view.
    [[nodiscard]] auto descriptor() const noexcept -> CpuDescriptor { return m_handle; }

private:
    /// @brief
    ///   The render device that this render target view is allocated from.
    RenderDevice *m_renderDevice;

    /// @brief
    ///   CPU descriptor handle of this render target view.
    CpuDescriptor m_handle;
};

class DepthStencilView {
private:
    /// @brief
    ///   For internal usage. Create a new depth stencil view.
    ///
    /// @param renderDevice
    ///   The render device that creates this depth stencil view.
    /// @param descriptor
    ///   The CPU descriptor handle for this depth stencil view.
    DepthStencilView(RenderDevice &renderDevice, CpuDescriptor descriptor) noexcept
        : m_renderDevice(&renderDevice), m_handle(descriptor) {}

    friend class RenderDevice;

public:
    /// @brief
    ///   Create a null depth stencil view.
    InkExport DepthStencilView() noexcept;

    /// @brief
    ///   Copy constructor of depth stencil view is disabled.
    DepthStencilView(const DepthStencilView &) = delete;

    /// @brief
    ///   Move constructor of depth stencil view.
    ///
    /// @param other
    ///   The depth stencil view to be moved. The moved depth stencil view will be set to null.
    InkExport DepthStencilView(DepthStencilView &&other) noexcept;

    /// @brief
    ///   Destroy this depth stencil view and release the CPU descriptor.
    InkExport ~DepthStencilView() noexcept;

    /// @brief
    ///   Copy assignment of depth stencil view is disabled.
    auto operator=(const DepthStencilView &) = delete;

    /// @brief
    ///   Move assignment of depth stencil view.
    ///
    /// @param other
    ///   The depth stencil view to be moved. The moved depth stencil view will be set to null.
    ///
    /// @return
    ///   Reference to this depth stencil view.
    InkExport auto operator=(DepthStencilView &&other) noexcept -> DepthStencilView &;

    /// @brief
    ///   Update this depth stencil view.
    /// @note
    ///   Null depth stencil view cannot be updated.
    ///
    /// @param resource
    ///   The resource that this depth stencil view is created from.
    /// @param desc
    ///   A description structure that describes how to create this depth stencil view. @p resource
    ///   must be non-null if @p desc is null.
    InkExport auto update(ID3D12Resource                      *resource,
                          const D3D12_DEPTH_STENCIL_VIEW_DESC *desc) noexcept -> void;

    /// @brief
    ///   Checks if this is a null depth stencil view.
    ///
    /// @return
    ///   A boolean value that indicates whether this is a null depth stencil view.
    /// @retval true
    ///   This is a null depth stencil view.
    /// @retval false
    ///   This is not a null depth stencil view.
    [[nodiscard]] auto isNull() const noexcept -> bool { return m_handle.isNull(); }

    /// @brief
    ///   Get CPU descriptor handle of this depth stencil view.
    ///
    /// @return
    ///   CPU descriptor handle of this depth stencil view.
    [[nodiscard]] auto descriptor() const noexcept -> CpuDescriptor { return m_handle; }

private:
    /// @brief
    ///   The render device that this depth stencil view is allocated from.
    RenderDevice *m_renderDevice;

    /// @brief
    ///   CPU descriptor handle of this depth stencil view.
    CpuDescriptor m_handle;
};

class Sampler {
private:
    /// @brief
    ///   For internal usage. Create a new sampler.
    ///
    /// @param renderDevice
    ///   The render device that creates this sampler.
    /// @param desc
    ///   A description structure that describes how to create this sampler.
    /// @param descriptor
    ///   The CPU descriptor handle for this sampler.
    Sampler(RenderDevice             &renderDevice,
            const D3D12_SAMPLER_DESC &desc,
            CpuDescriptor             descriptor) noexcept
        : m_renderDevice(&renderDevice), m_handle(descriptor), m_desc(desc) {}

    friend class RenderDevice;

public:
    /// @brief
    ///   Create a null sampler.
    InkExport Sampler() noexcept;

    /// @brief
    ///   Copy constructor of sampler is disabled.
    Sampler(const Sampler &) = delete;

    /// @brief
    ///   Move constructor of sampler.
    ///
    /// @param other
    ///   The sampler to be moved. The moved sampler will be set to null.
    InkExport Sampler(Sampler &&other) noexcept;

    /// @brief
    ///   Destroy this sampler and release the CPU descriptor.
    InkExport ~Sampler() noexcept;

    /// @brief
    ///   Copy assignment of sampler is disabled.
    auto operator=(const Sampler &) = delete;

    /// @brief
    ///   Move assignment of sampler.
    ///
    /// @param other
    ///   The sampler to be moved. The moved sampler will be set to null.
    ///
    /// @return
    ///   Reference to this sampler.
    InkExport auto operator=(Sampler &&other) noexcept -> Sampler &;

    /// @brief
    ///   Update this sampler.
    /// @note
    ///   Null sampler cannot be updated.
    ///
    /// @param desc
    ///   A description structure that describes how to create this sampler.
    InkExport auto update(const D3D12_SAMPLER_DESC &desc) noexcept -> void;

    /// @brief
    ///   Checks if this is a null sampler.
    ///
    /// @return
    ///   A boolean value that indicates whether this is a null sampler.
    /// @retval true
    ///   This is a null sampler.
    /// @retval false
    ///   This is not a null sampler.
    [[nodiscard]] auto isNull() const noexcept -> bool { return m_handle.isNull(); }

    /// @brief
    ///   Get CPU descriptor handle of this sampler.
    ///
    /// @return
    ///   CPU descriptor handle of this sampler.
    [[nodiscard]] auto descriptor() const noexcept -> CpuDescriptor { return m_handle; }

    /// @brief
    ///   Get D3D12 sampler desc of this sampler.
    ///
    /// @return
    ///   D3D12 sampler desc of this sampler.
    [[nodiscard]] auto samplerDesc() const noexcept -> const D3D12_SAMPLER_DESC & { return m_desc; }

    /// @brief
    ///   Get filter of this sampler.
    ///
    /// @return
    ///   Filter of this sampler.
    [[nodiscard]] auto filter() const noexcept -> D3D12_FILTER { return m_desc.Filter; }

    /// @brief
    ///   Get address mode along U axis of this sampler.
    ///
    /// @return
    ///   Address mode along U axis of this sampler.
    [[nodiscard]] auto addressModeU() const noexcept -> D3D12_TEXTURE_ADDRESS_MODE {
        return m_desc.AddressU;
    }

    /// @brief
    ///   Get address mode along V axis of this sampler.
    ///
    /// @return
    ///   Address mode along V axis of this sampler.
    [[nodiscard]] auto addressModeV() const noexcept -> D3D12_TEXTURE_ADDRESS_MODE {
        return m_desc.AddressV;
    }

    /// @brief
    ///   Get address mode along W axis of this sampler.
    ///
    /// @return
    ///   Address mode along W axis of this sampler.
    [[nodiscard]] auto addressModeW() const noexcept -> D3D12_TEXTURE_ADDRESS_MODE {
        return m_desc.AddressW;
    }

private:
    /// @brief
    ///   The render device that this sampler is allocated from.
    RenderDevice *m_renderDevice;

    /// @brief
    ///   CPU descriptor handle of this sampler.
    CpuDescriptor m_handle;

    /// @brief
    ///   D3D12 sampler desc that describes attributes of this sampler.
    D3D12_SAMPLER_DESC m_desc;
};

} // namespace ink

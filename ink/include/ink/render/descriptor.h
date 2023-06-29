#pragma once

#include <d3d12.h>

#include <cstddef>
#include <cstdint>

namespace ink {

struct CpuDescriptorHandle : D3D12_CPU_DESCRIPTOR_HANDLE {
    /// @brief
    ///   Create a null CPU descriptor handle.
    constexpr CpuDescriptorHandle() noexcept : D3D12_CPU_DESCRIPTOR_HANDLE{SIZE_T(-1)} {}

    /// @brief
    ///   Create a CPU descriptor handle from another one.
    ///
    /// @param handle
    ///   The CPU descriptor handle to be created from.
    constexpr CpuDescriptorHandle(D3D12_CPU_DESCRIPTOR_HANDLE handle) noexcept
        : D3D12_CPU_DESCRIPTOR_HANDLE(handle) {}

    /// @brief
    ///   Checks if this CPU descriptor handle is a null handle.
    ///
    /// @return
    ///   A boolean value that indicates whether this CPU descriptor handle is null.
    /// @retval true
    ///   This is a null CPU descriptor handle.
    /// @retval false
    ///   This is not a null CPU descriptor handle.
    [[nodiscard]]
    constexpr auto isNull() const noexcept -> bool {
        return ptr == SIZE_T(-1);
    }

    /// @brief
    ///   Apply an offset to this CPU descriptor handle.
    ///
    /// @param off
    ///   The offset to be applied to this descriptor handle.
    ///
    /// @return
    ///   Reference to this CPU descriptor handle.
    constexpr auto operator+=(std::ptrdiff_t off) noexcept -> CpuDescriptorHandle & {
        if (isNull())
            return *this;

        ptr = SIZE_T(std::ptrdiff_t(ptr) + off);
        return *this;
    }

    /// @brief
    ///   Apply a negative offset to this CPU descriptor handle.
    ///
    /// @param off
    ///   The offset to be applied to this descriptor handle.
    ///
    /// @return
    ///   Reference to this CPU descriptor handle.
    constexpr auto operator-=(std::ptrdiff_t off) noexcept -> CpuDescriptorHandle {
        if (isNull())
            return *this;

        ptr = SIZE_T(std::ptrdiff_t(ptr) - off);
        return *this;
    }
};

constexpr auto operator==(CpuDescriptorHandle lhs, CpuDescriptorHandle rhs) noexcept -> bool {
    return lhs.ptr == rhs.ptr;
}

constexpr auto operator!=(CpuDescriptorHandle lhs, CpuDescriptorHandle rhs) noexcept -> bool {
    return lhs.ptr != rhs.ptr;
}

constexpr auto operator<(CpuDescriptorHandle lhs, CpuDescriptorHandle rhs) noexcept -> bool {
    return lhs.ptr < rhs.ptr;
}

constexpr auto operator<=(CpuDescriptorHandle lhs, CpuDescriptorHandle rhs) noexcept -> bool {
    return lhs.ptr <= rhs.ptr;
}

constexpr auto operator>(CpuDescriptorHandle lhs, CpuDescriptorHandle rhs) noexcept -> bool {
    return lhs.ptr > rhs.ptr;
}

constexpr auto operator>=(CpuDescriptorHandle lhs, CpuDescriptorHandle rhs) noexcept -> bool {
    return lhs.ptr >= rhs.ptr;
}

constexpr auto operator+(CpuDescriptorHandle lhs, std::ptrdiff_t rhs) noexcept
    -> CpuDescriptorHandle {
    lhs += rhs;
    return lhs;
}

constexpr auto operator+(std::ptrdiff_t lhs, CpuDescriptorHandle rhs) noexcept
    -> CpuDescriptorHandle {
    rhs += lhs;
    return rhs;
}

constexpr auto operator-(CpuDescriptorHandle lhs, std::ptrdiff_t rhs) noexcept
    -> CpuDescriptorHandle {
    lhs -= rhs;
    return lhs;
}

class ConstantBufferView {
public:
    /// @brief
    ///   Create a null constant buffer view.
    ConstantBufferView() noexcept : m_handle() {}

    /// @brief
    ///   Copy constructor of constant buffer view.
    /// @note
    ///   Copying constant buffer view may need to allocate a free descriptor handle and then copy
    ///   the descriptor content which may take a long time.
    ///
    /// @param other
    ///   The constant buffer view to be copied from.
    ConstantBufferView(const ConstantBufferView &other) noexcept;

    /// @brief
    ///   Copy assignment of constant buffer view.
    /// @note
    ///   Copying constant buffer view may need to allocate a free descriptor handle and then copy
    ///   the descriptor content which may take a long time.
    ///
    /// @param other
    ///   The constant buffer view to be copied from.
    ///
    /// @return
    ///   Reference to this constant buffer view.
    auto operator=(const ConstantBufferView &other) noexcept -> ConstantBufferView &;

    /// @brief
    ///   Move constructor of constant buffer view.
    ///
    /// @param other
    ///   The constant buffer view to be moved. The moved constant buffer view will be invalidated.
    ConstantBufferView(ConstantBufferView &&other) noexcept : m_handle(other.m_handle) {
        other.m_handle = CpuDescriptorHandle();
    }

    /// @brief
    ///   Move assignment of constant buffer view.
    ///
    /// @param other
    ///   The constant buffer view to be moved. The moved constant buffer view will be invalidated.
    ///
    /// @return
    ///   Reference to this constant buffer view.
    auto operator=(ConstantBufferView &&other) noexcept -> ConstantBufferView &;

    /// @brief
    ///   Destroy this constant buffer view and free the descriptor handle.
    ~ConstantBufferView() noexcept;

    /// @brief
    ///   Create a constant buffer view in place.
    /// @note
    ///   A free CPU descriptor will be allocated if this is a null constant buffer view.
    ///
    /// @param gpuAddress
    ///   GPU address of the constant buffer view.
    /// @param size
    ///   Size in byte of the constant buffer view.
    auto initConstantBuffer(std::uint64_t gpuAddress, std::uint32_t size) noexcept -> void;

    /// @brief
    ///   Create a constant buffer view in place.
    /// @note
    ///   A free CPU descriptor will be allocated if this is a null constant buffer view.
    ///
    /// @param desc
    ///   A description structure that describes how to create the constant buffer view.
    auto initConstantBuffer(const D3D12_CONSTANT_BUFFER_VIEW_DESC &desc) noexcept -> void;

    /// @brief
    ///   Create a shader resource view in place.
    /// @note
    ///   A free CPU descriptor will be allocated if this is a null shader resource view.
    ///
    /// @param[in] resource
    ///   Nullable. The D3D12 resource that this shader resource view belongs to. @p desc must be
    ///   non-null if @p resource is null.
    /// @param desc
    ///   Nullable. A description structure that describes how to create this shader resource view.
    ///   @p resource must be non-null if @p desc is null.
    auto initShaderResource(ID3D12Resource                        *resource,
                            const D3D12_SHADER_RESOURCE_VIEW_DESC *desc) noexcept -> void;

    /// @brief
    ///   Create an unordered access view in place.
    /// @note
    ///   A free CPU descriptor will be allocated if this is a null unordered access view.
    ///
    /// @param[in] resource
    ///   Nullable. The D3D12 resource that this unordered access view belongs to. @p desc must be
    ///   non-null if @p resource is null.
    /// @param counter
    ///   Nullable. Counter resource of this unordered access view.
    /// @param desc
    ///   Nullable. A description structure that describes how to create this unordered access view.
    ///   @p resource must be non-null if @p desc is null.
    auto initUnorderedAccess(ID3D12Resource                         *resource,
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
    [[nodiscard]]
    auto isNull() const noexcept -> bool {
        return m_handle.isNull();
    }

    /// @brief
    ///   Get CPU descriptor handle of this constant buffer view.
    ///
    /// @return
    ///   CPU descriptor handle of this constant buffer view.
    [[nodiscard]]
    auto descriptorHandle() const noexcept -> CpuDescriptorHandle {
        return m_handle;
    }

    /// @brief
    ///   Allow implicit conversion to CPU descriptor handle.
    operator CpuDescriptorHandle() const noexcept {
        return m_handle;
    }

private:
    /// @brief
    ///   CPU descriptor handle of this constant buffer view.
    CpuDescriptorHandle m_handle;
};

/// @brief
///   Constant buffer view descriptors could be used as shader resource views in D3D12.
using ShaderResourceView = ConstantBufferView;

/// @brief
///   Constant buffer view descriptors could be used as unordered access views in D3D12.
using UnorderedAccessView = ConstantBufferView;

class SamplerView {
public:
    /// @brief
    ///   Create a null sampler view.
    SamplerView() noexcept : m_handle() {}

    /// @brief
    ///   Copy constructor of sampler view.
    /// @note
    ///   Copying sampler view may need to allocate a free descriptor handle and then copy the
    ///   descriptor content which may take a long time.
    ///
    /// @param other
    ///   The sampler view to be copied from.
    SamplerView(const SamplerView &other) noexcept;

    /// @brief
    ///   Copy assignment of sampler view.
    /// @note
    ///   Copying sampler view may need to allocate a free descriptor handle and then copy the
    ///   descriptor content which may take a long time.
    ///
    /// @param other
    ///   The sampler view to be copied from.
    ///
    /// @return
    ///   Reference to this sampler view.
    auto operator=(const SamplerView &other) noexcept -> SamplerView &;

    /// @brief
    ///   Move constructor of sampler view.
    ///
    /// @param other
    ///   The sampler view to be moved. The moved sampler view will be invalidated.
    SamplerView(SamplerView &&other) noexcept : m_handle(other.m_handle) {
        other.m_handle = CpuDescriptorHandle();
    }

    /// @brief
    ///   Move assignment of sampler view.
    ///
    /// @param other
    ///   The sampler view to be moved. The moved sampler view will be invalidated.
    ///
    /// @return
    ///   Reference to this sampler view.
    auto operator=(SamplerView &&other) noexcept -> SamplerView &;

    /// @brief
    ///   Destroy this sampler view and free the descriptor handle.
    ~SamplerView() noexcept;

    /// @brief
    ///   Create a new sampler view in place.
    /// @note
    ///   A free CPU descriptor will be allocated if this is a null sampler view.
    ///
    /// @param desc
    ///   A description structure that describes how to create this sampler view.
    auto initSampler(const D3D12_SAMPLER_DESC &desc) noexcept -> void;

    /// @brief
    ///   Checks if this is a null sampler view.
    ///
    /// @return
    ///   A boolean value that indicates whether this is a null sampler view.
    /// @retval true
    ///   This is a null sampler view.
    /// @retval false
    ///   This is not a null sampler view.
    [[nodiscard]]
    auto isNull() const noexcept -> bool {
        return m_handle.isNull();
    }

    /// @brief
    ///   Get CPU descriptor handle of this sampler view.
    ///
    /// @return
    ///   CPU descriptor handle of this sampler view.
    [[nodiscard]]
    auto descriptorHandle() const noexcept -> CpuDescriptorHandle {
        return m_handle;
    }

    /// @brief
    ///   Allow implicit conversion to CPU descriptor handle.
    operator CpuDescriptorHandle() const noexcept {
        return m_handle;
    }

private:
    /// @brief
    ///   CPU descriptor of this sampler view.
    CpuDescriptorHandle m_handle;
};

class RenderTargetView {
public:
    /// @brief
    ///   Create a null render target view.
    RenderTargetView() noexcept : m_handle() {}

    /// @brief
    ///   Copy constructor of render target view.
    /// @note
    ///   Copying render target view may need to allocate a free descriptor handle and then copy the
    ///   descriptor content which may take a long time.
    ///
    /// @param other
    ///   The render target view to be copied from.
    RenderTargetView(const RenderTargetView &other) noexcept;

    /// @brief
    ///   Copy assignment of render target view.
    /// @note
    ///   Copying render target view may need to allocate a free descriptor handle and then copy the
    ///   descriptor content which may take a long time.
    ///
    /// @param other
    ///   The render target view to be copied from.
    ///
    /// @return
    ///   Reference to this render target view.
    auto operator=(const RenderTargetView &other) noexcept -> RenderTargetView &;

    /// @brief
    ///   Move constructor of render target view.
    ///
    /// @param other
    ///   The render target view to be moved. The moved render target view will be invalidated.
    RenderTargetView(RenderTargetView &&other) noexcept : m_handle(other.m_handle) {
        other.m_handle = CpuDescriptorHandle();
    }

    /// @brief
    ///   Move assignment of render target view.
    ///
    /// @param other
    ///   The render target view to be moved. The moved render target view will be invalidated.
    ///
    /// @return
    ///   Reference to this render target view.
    auto operator=(RenderTargetView &&other) noexcept -> RenderTargetView &;

    /// @brief
    ///   Destroy this render target view and free the descriptor handle.
    ~RenderTargetView() noexcept;

    /// @brief
    ///   Create a render target view in place.
    /// @note
    ///   A free CPU descriptor will be allocated if this is a null render target view.
    ///
    /// @param[in] resource
    ///   Nullable. The D3D12 resource that this render target view belongs to. @p desc must be
    ///   non-null if @p resource is null.
    /// @param desc
    ///   Nullable. A description structure that describes how to create this render target view. @p
    ///   resource must be non-null if @p desc is null.
    auto initRenderTarget(ID3D12Resource                      *resource,
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
    [[nodiscard]]
    auto isNull() const noexcept -> bool {
        return m_handle.isNull();
    }

    /// @brief
    ///   Get CPU descriptor handle of this render target view.
    ///
    /// @return
    ///   CPU descriptor handle of this render target view.
    [[nodiscard]]
    auto descriptorHandle() const noexcept -> CpuDescriptorHandle {
        return m_handle;
    }

    /// @brief
    ///   Allow implicit conversion to CPU descriptor handle.
    operator CpuDescriptorHandle() const noexcept {
        return m_handle;
    }

private:
    /// @brief
    ///   CPU descriptor of this render target view.
    CpuDescriptorHandle m_handle;
};

class DepthStencilView {
public:
    /// @brief
    ///   Create a null depth stencil view.
    DepthStencilView() noexcept : m_handle() {}

    /// @brief
    ///   Copy constructor of depth stencil view.
    /// @note
    ///   Copying depth stencil view may need to allocate a free descriptor handle and then copy the
    ///   descriptor content which may take a long time.
    ///
    /// @param other
    ///   The depth stencil view to be copied from.
    DepthStencilView(const DepthStencilView &other) noexcept;

    /// @brief
    ///   Copy assignment of depth stencil view.
    /// @note
    ///   Copying depth stencil view may need to allocate a free descriptor handle and then copy the
    ///   descriptor content which may take a long time.
    ///
    /// @param other
    ///   The depth stencil view to be copied from.
    ///
    /// @return
    ///   Reference to this depth stencil view.
    auto operator=(const DepthStencilView &other) noexcept -> DepthStencilView &;

    /// @brief
    ///   Move constructor of depth stencil view.
    ///
    /// @param other
    ///   The depth stencil view to be moved. The moved depth stencil view will be invalidated.
    DepthStencilView(DepthStencilView &&other) noexcept : m_handle(other.m_handle) {
        other.m_handle = CpuDescriptorHandle();
    }

    /// @brief
    ///   Move assignment of depth stencil view.
    ///
    /// @param other
    ///   The depth stencil view to be moved. The moved depth stencil view will be invalidated.
    ///
    /// @return
    ///   Reference to this depth stencil view.
    auto operator=(DepthStencilView &&other) noexcept -> DepthStencilView &;

    /// @brief
    ///   Destroy this depth stencil view and free the descriptor handle.
    ~DepthStencilView() noexcept;

    /// @brief
    ///   Create a depth stencil view in place.
    /// @note
    ///   A free CPU descriptor will be allocated if this is a null depth stencil view.
    ///
    /// @param[in] resource
    ///   Nullable. The D3D12 resource that this depth stencil view belongs to. @p desc must be
    ///   non-null if @p resource is null.
    /// @param desc
    ///   Nullable. A description structure that describes how to create this depth stencil view. @p
    ///   resource must be non-null if @p desc is null.
    auto initDepthStencil(ID3D12Resource                      *resource,
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
    [[nodiscard]]
    auto isNull() const noexcept -> bool {
        return m_handle.isNull();
    }

    /// @brief
    ///   Get CPU descriptor handle of this depth stencil view.
    ///
    /// @return
    ///   CPU descriptor handle of this depth stencil view.
    [[nodiscard]]
    auto descriptorHandle() const noexcept -> CpuDescriptorHandle {
        return m_handle;
    }

    /// @brief
    ///   Allow implicit conversion to CPU descriptor handle.
    operator CpuDescriptorHandle() const noexcept {
        return m_handle;
    }

private:
    /// @brief
    ///   CPU descriptor of this depth stencil view.
    CpuDescriptorHandle m_handle;
};

} // namespace ink

#pragma once

#include "ink/render/color.h"
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

    /// @brief
    ///   Destroy this GPU buffer.
    ~GpuBuffer() noexcept;

    /// @brief
    ///   Get size in byte of this GPU buffer.
    ///
    /// @return
    ///   Size in byte of this GPU buffer.
    [[nodiscard]]
    auto size() const noexcept -> std::size_t {
        return m_size;
    }

    /// @brief
    ///   Get GPU virtual address to start of this GPU buffer.
    ///
    /// @return
    ///   GPU virtual address to start of this GPU buffer.
    [[nodiscard]]
    auto gpuAddress() const noexcept -> std::uint64_t {
        return m_gpuAddress;
    }

    /// @brief
    ///   Get byte address unordered access view CPU descriptor handle of this GPU buffer.
    ///
    /// @return
    ///   Byte address unordered access view CPU descriptor handle of this GPU buffer.
    [[nodiscard]]
    auto byteAddressUnorderedAccessView() const noexcept -> CpuDescriptorHandle {
        return m_byteAddressUAV;
    }

protected:
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

class StructuredBuffer : public GpuBuffer {
public:
    /// @brief
    ///   Create an empty structured buffer.
    StructuredBuffer() noexcept;

    /// @brief
    ///   Create new structured buffer for the specified kind of element.
    /// @note
    ///   Errors are handled with assertions.
    ///
    /// @param count
    ///   Number of elements to be stored in this structured buffer.
    /// @param size
    ///   Size in byte of each element.
    StructuredBuffer(std::uint32_t count, std::uint32_t size) noexcept;

    /// @brief
    ///   Move constructor of structured buffer.
    ///
    /// @param other
    ///   The structured buffer to be moved from. The moved structured buffer will be invalidated.
    StructuredBuffer(StructuredBuffer &&other) noexcept;

    /// @brief
    ///   Move assignment of structured buffer.
    ///
    /// @param other
    ///   The structured buffer to be moved from. The moved structured buffer will be invalidated.
    ///
    /// @return
    ///   Reference to this structured buffer.
    auto operator=(StructuredBuffer &&other) noexcept -> StructuredBuffer &;

    /// @brief
    ///   Destroy this structured buffer.
    ~StructuredBuffer() noexcept override;

    /// @brief
    ///   Get number of elements that could be stored in this structured buffer.
    ///
    /// @return
    ///   Number of elements that could be stored in this structured buffer.
    [[nodiscard]]
    auto elementCount() const noexcept -> std::uint32_t {
        return m_elementCount;
    }

    /// @brief
    ///   Get size in byte of each element.
    ///
    /// @return
    ///   Size in byte of each element.
    [[nodiscard]]
    auto elementSize() const noexcept -> std::uint32_t {
        return m_elementSize;
    }

    /// @brief
    ///   Reshape structure size and count for this structured buffer.
    /// @note
    ///   The GPU buffer will be recreated if the new required size is greater than the original GPU
    ///   buffer size. GPU address and buffer size will also be changed if GPU buffer is recreated.
    ///   Recreating GPU buffer may take a lot of time.
    ///
    /// @param newCount
    ///   New number of elements to be stored in this structured buffer.
    /// @param newSize
    ///   New size in byte of element.
    auto reshape(std::uint32_t newCount, std::uint32_t newSize) noexcept -> void;

    /// @brief
    ///   Get structured buffer unordered access view CPU descriptor handle of this GPU buffer.
    ///
    /// @return
    ///   Structured buffer unordered access view CPU descriptor handle of this GPU buffer.
    [[nodiscard]]
    auto structuredUnorderedAccessView() const noexcept -> CpuDescriptorHandle {
        return m_structuredBufferUAV;
    }

protected:
    /// @brief
    ///   Number of elements in this structured buffer.
    std::uint32_t m_elementCount;

    /// @brief
    ///   Size in byte of each element.
    std::uint32_t m_elementSize;

    /// @brief
    ///   Structured buffer unordered access view of this GPU buffer.
    UnorderedAccessView m_structuredBufferUAV;
};

class PixelBuffer : public GpuResource {
public:
    /// @brief
    ///   Create an empty pixel buffer.
    PixelBuffer() noexcept;

    /// @brief
    ///   Move constructor of pixel buffer.
    ///
    /// @param other
    ///   The pixel buffer to be moved. The moved pixel buffer will be invalidated.
    PixelBuffer(PixelBuffer &&other) noexcept;

    /// @brief
    ///   Move assignment of pixel buffer.
    ///
    /// @param other
    ///   The pixel buffer to be moved. The moved pixel buffer will be invalidated.
    ///
    /// @return
    ///   Reference to this pixel buffer.
    auto operator=(PixelBuffer &&other) noexcept -> PixelBuffer &;

    /// @brief
    ///   Destroy this pixel buffer.
    ~PixelBuffer() noexcept override;

    /// @brief
    ///   Get width in pixel of this pixel buffer.
    ///
    /// @return
    ///   Width in pixel of this pixel buffer.
    [[nodiscard]]
    auto width() const noexcept -> std::uint32_t {
        return m_width;
    }

    /// @brief
    ///   Get height in pixel of this pixel buffer.
    ///
    /// @return
    ///   Height in pixel of this pixel buffer.
    [[nodiscard]]
    auto height() const noexcept -> std::uint32_t {
        return m_height;
    }

    /// @brief
    ///   Get number of sample points per-pixel of this pixel buffer.
    ///
    /// @return
    ///   Number of sample points per-pixel of this pixel buffer.
    [[nodiscard]]
    auto arraySize() const noexcept -> std::uint32_t {
        return m_arraySize;
    }

    /// @brief
    ///   Get number of sample points per-pixel of this pixel buffer.
    ///
    /// @return
    ///   Number of sample points per-pixel of this pixel buffer.
    [[nodiscard]]
    auto sampleCount() const noexcept -> std::uint32_t {
        return m_sampleCount;
    }

    /// @brief
    ///   Get maximum supported mipmap levels by this pixel buffer.
    ///
    /// @return
    ///   Maximum supported mipmap levels by this pixel buffer.
    [[nodiscard]]
    auto mipLevels() const noexcept -> std::uint32_t {
        return m_mipLevels;
    }

    /// @brief
    ///   Get pixel format of this pixel buffer.
    ///
    /// @return
    ///   Pixel format of this pixel buffer.
    [[nodiscard]]
    auto pixelFormat() const noexcept -> DXGI_FORMAT {
        return m_pixelFormat;
    }

protected:
    /// @brief
    ///   Width in pixel of this pixel buffer.
    std::uint32_t m_width;

    /// @brief
    ///   Height in pixel of this pixel buffer.
    std::uint32_t m_height;

    /// @brief
    ///   Number of 2D textures in this pixel buffer.
    std::uint32_t m_arraySize;

    /// @brief
    ///   Number of sample points per-pixel of this pixel buffer.
    std::uint32_t m_sampleCount;

    /// @brief
    ///   Maximum supported mipmap levels by this pixel buffer.
    std::uint32_t m_mipLevels;

    /// @brief
    ///   Pixel format of this pixel buffer.
    DXGI_FORMAT m_pixelFormat;
};

class ColorBuffer : public PixelBuffer {
public:
    /// @brief
    ///   Create an empty color buffer.
    ColorBuffer() noexcept;

    /// @brief
    ///   Create a new color buffer.
    /// @note
    ///   Errors are handled with assertions.
    ///
    /// @param width
    ///   Width in pixel of this color buffer.
    /// @param height
    ///   Height in pixel of this color buffer.
    /// @param arraySize
    ///   Number of 2D textures in this color buffer if this is a 2D texture array.
    /// @param format
    ///   Pixel format of this color buffer.
    /// @param mipLevels
    ///   Maximum supported mipmap levels by this color buffer. This value will always be clamped
    ///   between 1 and maximum supported levels. Pass 0 to use maximum supported mipmap level.
    /// @param sampleCount
    ///   Number of samples per-pixel of this color buffer. This value will be set to 1 if 0 is
    ///   passed. Enabling multi-sampling will disable unordered access for this color buffer.
    ColorBuffer(std::uint32_t width,
                std::uint32_t height,
                std::uint32_t arraySize,
                DXGI_FORMAT   format,
                std::uint32_t mipLevels   = 1,
                std::uint32_t sampleCount = 1) noexcept;

    /// @brief
    ///   Create a new 2D color buffer.
    /// @note
    ///   Errors are handled with assertions.
    ///
    /// @param width
    ///   Width in pixel of this color buffer.
    /// @param height
    ///   Height in pixel of this color buffer.
    /// @param format
    ///   Pixel format of this color buffer.
    ColorBuffer(std::uint32_t width, std::uint32_t height, DXGI_FORMAT format) noexcept
        : ColorBuffer(width, height, 1, format, 1, 1) {}

    /// @brief
    ///   Move constructor of color buffer.
    ///
    /// @param other
    ///   The color buffer to be moved from. The moved color buffer will be invalidated.
    ColorBuffer(ColorBuffer &&other) noexcept;

    /// @brief
    ///   Move assignment of color buffer.
    ///
    /// @param other
    ///   The color buffer to be moved from. The moved color buffer will be invalidated.
    ///
    /// @return
    ///   Reference to this color buffer.
    auto operator=(ColorBuffer &&other) noexcept -> ColorBuffer &;

    /// @brief
    ///   Destroy this color buffer.
    ~ColorBuffer() noexcept override;

    /// @brief
    ///   Get clear color of this color buffer.
    ///
    /// @return
    ///   Clear color of this color buffer.
    [[nodiscard]]
    auto clearColor() const noexcept -> const Color & {
        return m_clearColor;
    }

    /// @brief
    ///   Set a new clear color for this color buffer.
    ///
    /// @param color
    ///   New clear color to be set.
    auto clearColor(const Color &color) noexcept -> void {
        m_clearColor = color;
    }

    /// @brief
    ///   Get render target view CPU descriptor handle of this color buffer.
    /// @remark
    ///   This render target view views the whole color buffer. It is guaranteed that all color
    ///   buffers have render target view.
    ///
    /// @return
    ///   Render target view CPU descriptor handle of this color buffer.
    [[nodiscard]]
    auto renderTargetView() const noexcept -> CpuDescriptorHandle {
        return m_rtv;
    }

    /// @brief
    ///   Get shader resource view CPU descriptor handle of this color buffer.
    /// @remark
    ///   This shader resource view views the whole color buffer.
    /// @note
    ///   Color buffers created by swap chain do not have shader resource view and this method will
    ///   return a null CPU descriptor handle.
    ///
    /// @return
    ///   Shader resource view CPU descriptor handle of this color buffer.
    [[nodiscard]]
    auto shaderResourceView() const noexcept -> CpuDescriptorHandle {
        return m_srv;
    }

    /// @brief
    ///   Get unordered access view CPU descriptor handle of this color buffer.
    /// @remark
    ///   This unordered access view views the whole color buffer.
    /// @note
    ///   Multi-sampling color buffer and some pixel formats do not support unordered access. Color
    ///   buffers created by swap chain do not support unordered access, too. A null CPU descriptor
    ///   handle will be returned if unordered access is not supported.
    ///
    /// @return
    ///   Unordered access view CPU descriptor handle of this color buffer.
    [[nodiscard]]
    auto unorderedAccessView() const noexcept -> CpuDescriptorHandle {
        return m_uav;
    }

    friend class SwapChain;

private:
    /// @brief
    ///   For swap chain usage. Release ID3D12Resource so that swap chain could resize back buffers.
    auto releaseSwapChainResource() noexcept -> void {
        m_resource.Reset();
    }

    /// @brief
    ///   For swap chain usage. Reset back buffer D3D12 resource.
    /// @note
    ///   Errors are handled with assertions.
    ///
    /// @param buffer
    ///   The new swap chain back buffer resource to be set.
    auto resetSwapChainBuffer(Microsoft::WRL::ComPtr<ID3D12Resource> buffer) noexcept -> void;

protected:
    /// @brief
    ///   Clear color of this color buffer.
    Color m_clearColor;

    /// @brief
    ///   Render target view of this color buffer. The whole color buffer could be viewed by this
    ///   render target view.
    RenderTargetView m_rtv;

    /// @brief
    ///   Shader resource view of this color buffer. Color buffers created by swap chain do not
    ///   support shader resource.
    ShaderResourceView m_srv;

    /// @brief
    ///   Unordered access view of this color buffer. Not all pixel formats supports unordered
    ///   access and color buffers created by swap chain do not support unordered access.
    UnorderedAccessView m_uav;
};

} // namespace ink

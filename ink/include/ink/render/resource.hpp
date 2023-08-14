#pragma once

#include "descriptor.hpp"

#include <wrl/client.h>

namespace ink {

struct Color {
    float red   = 0;
    float green = 0;
    float blue  = 0;
    float alpha = 0;

    /// @brief
    ///   Create a transparent black color. All elements are initialized to 0.
    constexpr Color() noexcept = default;

    /// @brief
    ///   Create a color with the given color values.
    ///
    /// @param r
    ///   Red intensity of this color. For non-HDR images, this value should be less than or equal
    ///   to 1.
    /// @param g
    ///   Green intensity of this color. For non-HDR images, this value should be less than or equal
    ///   to 1.
    /// @param b
    ///   Blue intensity of this color. For non-HDR images, this value should be less than or equal
    ///   to 1.
    /// @param a
    ///   Opacity of this color.
    constexpr Color(float r, float g, float b, float a) noexcept
        : red(r), green(g), blue(b), alpha(a) {}
};

namespace colors {

inline constexpr Color Transparent{0.0f, 0.0f, 0.0f, 0.0f};
inline constexpr Color Black{0.0f, 0.0f, 0.0f, 1.0f};
inline constexpr Color White{1.0f, 1.0f, 1.0f, 1.0f};
inline constexpr Color Red{1.0f, 0.0f, 0.0f, 1.0f};
inline constexpr Color Green{0.0f, 1.0f, 0.0f, 1.0f};
inline constexpr Color Blue{0.0f, 0.0f, 1.0f, 1.0f};

} // namespace colors

class GpuResource {
public:
    /// @brief
    ///   Create an empty GPU resource.
    InkExport GpuResource() noexcept;

    /// @brief
    ///   Copy constructor of GPU resource is disabled.
    GpuResource(const GpuResource &) = delete;

    /// @brief
    ///   Move constructor of GPU resource.
    ///
    /// @param other
    ///   The GPU resource object to be moved. The moved GPU resource will be invalidated.
    InkExport GpuResource(GpuResource &&other) noexcept;

    /// @brief
    ///   Destroy this GPU resource.
    InkExport virtual ~GpuResource() noexcept;

    /// @brief
    ///   Copy assignment of GPU resource is disabled.
    auto operator=(const GpuResource &) = delete;

    /// @brief
    ///   Move assignment of GPU resource.
    ///
    /// @param other
    ///   The GPU resource object to be moved. The moved GPU resource will be invalidated.
    ///
    /// @return
    ///   Reference to this GPU resource.
    InkExport auto operator=(GpuResource &&other) noexcept -> GpuResource &;

    /// @brief
    ///   Get GPU usage state of this GPU resource.
    ///
    /// @return
    ///   GPU usage state of this GPU resource.
    [[nodiscard]] auto state() const noexcept -> D3D12_RESOURCE_STATES { return m_usageState; }

    friend class CommandBuffer;

protected:
    /// @brief
    ///   D3D12 resource object.
    Microsoft::WRL::ComPtr<ID3D12Resource> m_resource;

    /// @brief
    ///   Resource usage state.
    D3D12_RESOURCE_STATES m_usageState;
};

class RenderDevice;

class GpuBuffer : public GpuResource {
protected:
    /// @brief
    ///   For internal usage. Create a new GPU buffer with at least the specified size.
    ///
    /// @param renderDevice
    ///   The render device that is used to create this GPU buffer.
    /// @param device
    ///   The D3D12 device that is used to create D3D12 resources.
    /// @param size
    ///   Expected size in byte of this GPU buffer. The actual size may be greater due to alignment.
    ///
    /// @throw RenderAPIException
    ///   Thrown if failed to create D3D12 resource and views.
    GpuBuffer(RenderDevice &renderDevice, ID3D12Device *device, std::size_t size);

    friend class RenderDevice;

public:
    /// @brief
    ///   Create an empty GPU buffer.
    InkExport GpuBuffer() noexcept;

    /// @brief
    ///   Move constructor of GPU buffer.
    ///
    /// @param other
    ///   The GPU buffer to be moved. The moved GPU buffer will be invalidated.
    InkExport GpuBuffer(GpuBuffer &&other) noexcept;

    /// @brief
    ///   Destroy this GPU buffer.
    InkExport ~GpuBuffer() noexcept override;

    /// @brief
    ///   Move assignment of GPU buffer.
    ///
    /// @param other
    ///   The GPU buffer to be moved. The moved GPU buffer will be invalidated.
    ///
    /// @return
    ///   Reference to this GPU buffer.
    InkExport auto operator=(GpuBuffer &&other) noexcept -> GpuBuffer &;

    /// @brief
    ///   Get size in byte of this GPU buffer.
    ///
    /// @return
    ///   Size in byte of this GPU buffer.
    [[nodiscard]] auto size() const noexcept -> std::size_t { return m_size; }

    /// @brief
    ///   Get GPU virtual address to start of this GPU buffer.
    ///
    /// @return
    ///   GPU virtual address to start of this GPU buffer.
    [[nodiscard]] auto gpuAddress() const noexcept -> std::uint64_t { return m_gpuAddress; }

    /// @brief
    ///   Get byte address unordered access view CPU descriptor handle of this GPU buffer.
    ///
    /// @return
    ///   Byte address unordered access view CPU descriptor handle of this GPU buffer.
    [[nodiscard]] auto byteAddressUnorderedAccessView() const noexcept -> CpuDescriptor {
        return m_byteAddressUAV.descriptor();
    }

protected:
    /// @brief
    ///   Size in byte of this GPU buffer.
    std::size_t m_size;

    /// @brief
    ///   GPU virtual address of this GPU buffer.
    std::uint64_t m_gpuAddress;

    /// @brief
    ///   Byte address unordered access view of this GPU buffer. It is guaranteed that GPU buffers
    ///   support byte address unordered access.
    UnorderedAccessView m_byteAddressUAV;
};

class StructuredBuffer : public GpuBuffer {
protected:
    /// @brief
    ///   For internal usage. Create a new structured buffer for the specified kind of element.
    ///
    /// @param renderDevice
    ///   The render device that is used to create this GPU buffer.
    /// @param device
    ///   The D3D12 device that is used to create D3D12 resources.
    /// @param elementCount
    ///   Number of elements in this structured buffer.
    /// @param elementSize
    ///   Size in byte of each element in this structured buffer.
    ///
    /// @throw RenderAPIException
    ///   Thrown if failed to create D3D12 resource and views.
    StructuredBuffer(RenderDevice &renderDevice,
                     ID3D12Device *device,
                     std::uint32_t elementCount,
                     std::uint32_t elementSize);

    friend class RenderDevice;

public:
    /// @brief
    ///   Create an empty structured buffer.
    InkExport StructuredBuffer() noexcept;

    /// @brief
    ///   Move constructor of structured buffer.
    ///
    /// @param other
    ///   The structured buffer to be moved from. The moved structured buffer will be invalidated.
    InkExport StructuredBuffer(StructuredBuffer &&other) noexcept;

    /// @brief
    ///   Destroy this structured buffer.
    InkExport ~StructuredBuffer() noexcept override;

    /// @brief
    ///   Move assignment of structured buffer.
    ///
    /// @param other
    ///   The structured buffer to be moved from. The moved structured buffer will be invalidated.
    ///
    /// @return
    ///   Reference to this structured buffer.
    InkExport auto operator=(StructuredBuffer &&other) noexcept -> StructuredBuffer &;

    /// @brief
    ///   Get number of elements that could be stored in this structured buffer.
    ///
    /// @return
    ///   Number of elements that could be stored in this structured buffer.
    [[nodiscard]] auto elementCount() const noexcept -> std::uint32_t { return m_elementCount; }

    /// @brief
    ///   Get size in byte of each element.
    ///
    /// @return
    ///   Size in byte of each element.
    [[nodiscard]] auto elementSize() const noexcept -> std::uint32_t { return m_elementSize; }

    /// @brief
    ///   Get structured buffer unordered access view CPU descriptor handle of this GPU buffer.
    ///
    /// @return
    ///   Structured buffer unordered access view CPU descriptor handle of this GPU buffer.
    [[nodiscard]] auto structuredUnorderedAccessView() const noexcept -> CpuDescriptor {
        return m_structuredBufferUAV.descriptor();
    }

    /// @brief
    ///   Reshape element count and size for this structured buffer. The GPU buffer will be
    ///   recreated if the new required size is greater than current size. GPU address and buffer
    ///   size will then be updated. You should always assume that the buffer content is invalid
    ///   once reshaped. Empty structured buffer cannot be reshaped.
    ///
    /// @param newCount
    ///   New number of elements in this structured buffer.
    /// @param newSize
    ///   New size in byte of each element in this structured buffer.
    ///
    /// @throw RenderAPIException
    ///   Thrown if failed to recreate this structured buffer. The original buffer will not be
    ///   modified if failed to recreate.
    InkExport auto reshape(std::uint32_t newCount, std::uint32_t newSize) -> void;

protected:
    /// @brief
    ///   The D3D12 device that is used to create this GPU buffer. This is used to recreate the
    ///   structured buffer.
    ID3D12Device *m_device;

    /// @brief
    ///   Number of elements in this structured buffer.
    std::uint32_t m_elementCount;

    /// @brief
    ///   Size in byte of each element in this structured buffer.
    std::uint32_t m_elementSize;

    /// @brief
    ///   Structured buffer unordered access view of this GPU buffer.
    UnorderedAccessView m_structuredBufferUAV;
};

class PixelBuffer : public GpuResource {
public:
    /// @brief
    ///   Create an empty pixel buffer.
    InkExport PixelBuffer() noexcept;

    /// @brief
    ///   Move constructor of pixel buffer.
    ///
    /// @param other
    ///   The pixel buffer to be moved. The moved pixel buffer will be invalidated.
    InkExport PixelBuffer(PixelBuffer &&other) noexcept;

    /// @brief
    ///   Destroy this pixel buffer.
    InkExport ~PixelBuffer() noexcept override;

    /// @brief
    ///   Move assignment of pixel buffer.
    ///
    /// @param other
    ///   The pixel buffer to be moved. The moved pixel buffer will be invalidated.
    ///
    /// @return
    ///   Reference to this pixel buffer.
    InkExport auto operator=(PixelBuffer &&other) noexcept -> PixelBuffer &;

    /// @brief
    ///   Get width in pixel of this pixel buffer.
    ///
    /// @return
    ///   Width in pixel of this pixel buffer.
    [[nodiscard]] auto width() const noexcept -> std::uint32_t { return m_width; }

    /// @brief
    ///   Get height in pixel of this pixel buffer.
    ///
    /// @return
    ///   Height in pixel of this pixel buffer.
    [[nodiscard]] auto height() const noexcept -> std::uint32_t { return m_height; }

    /// @brief
    ///   Get number of sample points per-pixel of this pixel buffer.
    ///
    /// @return
    ///   Number of sample points per-pixel of this pixel buffer.
    [[nodiscard]] auto arraySize() const noexcept -> std::uint32_t { return m_arraySize; }

    /// @brief
    ///   Get number of sample points per-pixel of this pixel buffer.
    ///
    /// @return
    ///   Number of sample points per-pixel of this pixel buffer.
    [[nodiscard]] auto sampleCount() const noexcept -> std::uint32_t { return m_sampleCount; }

    /// @brief
    ///   Get maximum supported mipmap levels by this pixel buffer.
    ///
    /// @return
    ///   Maximum supported mipmap levels by this pixel buffer.
    [[nodiscard]] auto mipLevels() const noexcept -> std::uint32_t { return m_mipLevels; }

    /// @brief
    ///   Get pixel format of this pixel buffer.
    ///
    /// @return
    ///   Pixel format of this pixel buffer.
    [[nodiscard]] auto pixelFormat() const noexcept -> DXGI_FORMAT { return m_pixelFormat; }

    /// @brief
    ///   Get shader resource view CPU descriptor handle of this pixel buffer. Pixel buffers should
    ///   be able to be used as shader resources, except for color buffers created by swap chain.
    ///   For depth buffer, this is depth only shader resource view and only depth data could be
    ///   accessed.
    ///
    /// @return
    ///   Shader resource view CPU descriptor handle of this pixel buffer.
    [[nodiscard]] auto shaderResourceView() const noexcept -> CpuDescriptor {
        return m_shaderResourceView.descriptor();
    }

    /// @brief
    ///   Get unordered access view CPU descriptor handle of this pixel buffer. Not all pixel
    ///   buffers support unordered access view.
    ///
    /// @return
    ///   Unordered access view CPU descriptor handle of this pixel buffer.
    [[nodiscard]] auto unorderedAccessView() const noexcept -> CpuDescriptor {
        return m_unorderedAccessView.descriptor();
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
    ///   Number of samples per pixel in this pixel buffer.
    std::uint32_t m_sampleCount;

    /// @brief
    ///   Maximum supported mipmap levels by this pixel buffer.
    std::uint32_t m_mipLevels;

    /// @brief
    ///   Pixel format of this pixel buffer.
    DXGI_FORMAT m_pixelFormat;

    /// @brief
    ///   Shader resource view of this pixel buffer. Pixel buffers should be able to be used as
    ///   shader resources, except for color buffers created by swap chain.
    ShaderResourceView m_shaderResourceView;

    /// @brief
    ///   Unordered access view of this pixel buffer. Not all pixel formats support unordered
    ///   access. This will be null if unordered access is not supported.
    UnorderedAccessView m_unorderedAccessView;
};

class ColorBuffer : public PixelBuffer {
protected:
    /// @brief
    ///   For internal usage. Create a new color buffer.
    ///
    /// @param renderDevice
    ///   The render device that is used to create this color buffer.
    /// @param device
    ///   The D3D12 device that is used to create this color buffer.
    /// @param width
    ///   Width in pixel of this color buffer.
    /// @param height
    ///   Height in pixel of this color buffer.
    /// @param arraySize
    ///   Number of 2D textures in this color buffer.
    /// @param format
    ///   Pixel format of this color buffer.
    /// @param mipLevels
    ///   Maximum supported mipmap levels by this color buffer.
    /// @param sampleCount
    ///   Number of samples per pixel in this color buffer.
    ///
    /// @throw RenderAPIException
    ///   Thrown if failed to create this color buffer.
    ColorBuffer(RenderDevice &renderDevice,
                ID3D12Device *device,
                std::uint32_t width,
                std::uint32_t height,
                std::uint32_t arraySize,
                DXGI_FORMAT   format,
                std::uint32_t mipLevels,
                std::uint32_t sampleCount);

    friend class RenderDevice;

public:
    /// @brief
    ///   Create an empty color buffer.
    InkExport ColorBuffer() noexcept;

    /// @brief
    ///   Move constructor of color buffer.
    ///
    /// @param other
    ///   The color buffer to be moved from. The moved color buffer will be invalidated.
    InkExport ColorBuffer(ColorBuffer &&other) noexcept;

    /// @brief
    ///   Destroy this color buffer.
    InkExport ~ColorBuffer() noexcept override;

    /// @brief
    ///   Move assignment of color buffer.
    ///
    /// @param other
    ///   The color buffer to be moved from. The moved color buffer will be invalidated.
    ///
    /// @return
    ///   Reference to this color buffer.
    InkExport auto operator=(ColorBuffer &&other) noexcept -> ColorBuffer &;

    /// @brief
    ///   Get clear color of this color buffer.
    ///
    /// @return
    ///   Clear color of this color buffer.
    [[nodiscard]] auto clearColor() const noexcept -> const Color & { return m_clearColor; }

    /// @brief
    ///   Set a new clear color for this color buffer.
    ///
    /// @param color
    ///   New clear color to be set.
    auto clearColor(const Color &color) noexcept -> void { m_clearColor = color; }

    /// @brief
    ///   Get render target view CPU descriptor handle of this color buffer.
    /// @remark
    ///   This render target view views the whole color buffer. It is guaranteed that all color
    ///   buffers have render target view.
    ///
    /// @return
    ///   Render target view CPU descriptor handle of this color buffer.
    [[nodiscard]] auto renderTargetView() const noexcept -> CpuDescriptor {
        return m_renderTargetView.descriptor();
    }

    friend class SwapChain;

private:
    /// @brief
    ///   For swap chain usage. Release ID3D12Resource so that swap chain could resize back buffers.
    auto releaseSwapChainResource() noexcept -> void { m_resource.Reset(); }

    /// @brief
    ///   For swap chain usage. Reset back buffer D3D12 resource.
    ///
    /// @param buffer
    ///   The new swap chain back buffer resource to be set.
    auto resetSwapChainBuffer(Microsoft::WRL::ComPtr<ID3D12Resource> buffer) noexcept -> void;

protected:
    /// @brief
    ///   Clear color of this color buffer.
    Color m_clearColor;

    /// @brief
    ///   Render target view of this color buffer.
    RenderTargetView m_renderTargetView;
};

class DepthBuffer : public PixelBuffer {
protected:
    /// @brief
    ///   For internal usage. Create a new depth buffer.
    ///
    /// @param renderDevice
    ///   The render device that is used to create this depth buffer.
    /// @param device
    ///   The D3D12 device that is used to create this depth buffer.
    /// @param width
    ///   Width in pixel of this depth buffer.
    /// @param height
    ///   Height in pixel of this depth buffer.
    /// @param format
    ///   Pixel format of this depth buffer.
    /// @param sampleCount
    ///   Number of samples per pixel in this depth buffer.
    ///
    /// @throw RenderAPIException
    ///   Thrown if failed to create this depth buffer.
    DepthBuffer(RenderDevice &renderDevice,
                ID3D12Device *device,
                std::uint32_t width,
                std::uint32_t height,
                DXGI_FORMAT   format,
                std::uint32_t sampleCount);

    friend class RenderDevice;

public:
    /// @brief
    ///   Create an empty depth buffer.
    InkExport DepthBuffer() noexcept;

    /// @brief
    ///   Move constructor of depth buffer.
    ///
    /// @param other
    ///   The depth buffer to be moved from. The moved depth buffer will be invalidated.
    InkExport DepthBuffer(DepthBuffer &&other) noexcept;

    /// @brief
    ///   Destroy this depth buffer.
    InkExport ~DepthBuffer() noexcept override;

    /// @brief
    ///   Move assignment of depth buffer.
    ///
    /// @param other
    ///   The depth buffer to be moved from. The moved depth buffer will be invalidated.
    ///
    /// @return
    ///   Reference to this depth buffer.
    InkExport auto operator=(DepthBuffer &&other) noexcept -> DepthBuffer &;

    /// @brief
    ///   Get clear depth value of this depth buffer. Default clear depth value is 1.0f.
    ///
    /// @return
    ///   Clear depth value of this depth buffer.
    [[nodiscard]] auto clearDepth() const noexcept -> float { return m_clearDepth; }

    /// @brief
    ///   Set a new clear depth value for this depth buffer.
    ///
    /// @param depth
    ///   New depth to be set.
    auto clearDepth(float depth) noexcept -> void { m_clearDepth = depth; }

    /// @brief
    ///   Get clear stencil value of this depth buffer. Default clear stencil value is 0.
    ///
    /// @return
    ///   Clear stencil value of this depth buffer.
    [[nodiscard]] auto clearStencil() const noexcept -> std::uint8_t { return m_clearStencil; }

    /// @brief
    ///   Set a new clear stencil value for this depth buffer.
    ///
    /// @param stencil
    ///   New clear stencil value to be set.
    auto clearStencil(std::uint8_t stencil) noexcept -> void { m_clearStencil = stencil; }

    /// @brief
    ///   Get depth stencil view CPU descriptor handle of this depth buffer. It is guaranteed that
    ///   depth buffers always have depth stencil view.
    ///
    /// @return
    ///   CPU descriptor handle to the depth stencil view of this depth buffer.
    [[nodiscard]] auto depthStencilView() const noexcept -> CpuDescriptor {
        return m_depthStencilView.descriptor();
    }

    /// @brief
    ///   Get depth read-only depth stencil view CPU descriptor handle of this depth buffer. It is
    ///   guaranteed that depth buffers always have depth read-only depth stencil view.
    /// @remark
    ///   Both depth data and stencil data could be accessed via this depth stencil view, but depth
    ///   data is read-only.
    ///
    /// @return
    ///   CPU descriptor handle to the depth read-only depth stencil view.
    [[nodiscard]] auto depthReadOnlyDepthStencilView() const noexcept -> CpuDescriptor {
        return m_depthReadOnlyView.descriptor();
    }

protected:
    /// @brief
    ///   Clear depth of this depth buffer.
    float m_clearDepth;

    /// @brief
    ///   Clear stencil of this depth buffer.
    std::uint8_t m_clearStencil;

    /// @brief
    ///   Depth stencil view of this depth buffer.
    DepthStencilView m_depthStencilView;

    /// @brief
    ///   Depth read-only depth stencil view of this depth buffer.
    DepthStencilView m_depthReadOnlyView;
};

class Texture2D : public PixelBuffer {
protected:
    /// @brief
    ///   For internal usage. Create a new 2D texture (array).
    ///
    /// @param renderDevice
    ///   The render device that is used to create this texture.
    /// @param device
    ///   The D3D12 device that is used to create this texture.
    /// @param width
    ///   Width in pixel of this texture.
    /// @param height
    ///   Height in pixel of this texture.
    /// @param arraySize
    ///   Number of 2D textures in this texture array.
    /// @param format
    ///   Pixel format of this texture.
    /// @param mipLevels
    ///   Maximum available mipmap level for this 2D texture.
    /// @param isCube
    ///   Specifies whether this is a cube texture. This value is used only if @p arraySize is a
    ///   multiple of 6.
    ///
    /// @throw RenderAPIException
    ///   Thrown if failed to create D3D12 resource or view for this texture.
    Texture2D(RenderDevice &renderDevice,
              ID3D12Device *device,
              std::uint32_t width,
              std::uint32_t height,
              std::uint32_t arraySize,
              DXGI_FORMAT   format,
              std::uint32_t mipLevels,
              bool          isCube);

    friend class RenderDevice;

public:
    /// @brief
    ///   Create an empty 2D texture.
    InkExport Texture2D() noexcept;

    /// @brief
    ///   Move constructor of 2D texture.
    ///
    /// @param other
    ///   The 2D texture to be moved. The moved 2D texture will be invalidated.
    InkExport Texture2D(Texture2D &&other) noexcept;

    /// @brief
    ///   Destroy this 2D texture.
    InkExport ~Texture2D() noexcept override;

    /// @brief
    ///   Move assignment of 2D texture.
    ///
    /// @param other
    ///   The 2D texture to be moved. The moved 2D texture will be invalidated.
    ///
    /// @return
    ///   Reference to this 2D texture.
    InkExport auto operator=(Texture2D &&other) noexcept -> Texture2D &;

    /// @brief
    ///   Checks if this is a 2D cube texture.
    ///
    /// @return
    ///   A boolean value that indicates whether this is a 2D cube texture.
    /// @retval true
    ///   This is a 2D cube texture.
    /// @retval false
    ///   This is not a 2D cube texture.
    [[nodiscard]] auto isCubeTexture() const noexcept -> bool { return m_isCube; }

protected:
    /// @brief
    ///   Specifies whether this is a cube texture;
    bool m_isCube;
};

} // namespace ink

#include "ink/render/resource.h"
#include "ink/core/assert.h"
#include "ink/render/device.h"

using namespace ink;

// Prevent compiler from generating RTTI everywhere.
ink::GpuResource::~GpuResource() noexcept {}

ink::GpuBuffer::GpuBuffer() noexcept
    : GpuResource(), m_size(), m_gpuAddress(), m_byteAddressUAV() {}

ink::GpuBuffer::GpuBuffer(std::size_t size) noexcept
    : GpuResource(),
      m_size((size + 0xFF) & ~std::size_t(0xFF)),
      m_gpuAddress(),
      m_byteAddressUAV() {
    [[maybe_unused]] HRESULT hr;

    auto &dev = RenderDevice::singleton();

    { // Create ID3D12Resource.
        const D3D12_HEAP_PROPERTIES heapProps{
            /* Type                 = */ D3D12_HEAP_TYPE_DEFAULT,
            /* CPUPageProperty      = */ D3D12_CPU_PAGE_PROPERTY_UNKNOWN,
            /* MemoryPoolPreference = */ D3D12_MEMORY_POOL_UNKNOWN,
            /* CreationNodeMask     = */ 0,
            /* VisibleNodeMask      = */ 0,
        };

        const D3D12_RESOURCE_DESC desc{
            /* Dimension        = */ D3D12_RESOURCE_DIMENSION_BUFFER,
            /* Alignment        = */ 0,
            /* Width            = */ m_size,
            /* Height           = */ 1,
            /* DepthOrArraySize = */ 1,
            /* MipLevels        = */ 1,
            /* Format           = */ DXGI_FORMAT_UNKNOWN,
            /* SampleDesc       = */
            {
                /* Count   = */ 1,
                /* Quality = */ 0,
            },
            /* Layout = */ D3D12_TEXTURE_LAYOUT_ROW_MAJOR,
            /* Flags  = */ D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS,
        };

        hr =
            dev.device()->CreateCommittedResource(&heapProps, D3D12_HEAP_FLAG_NONE, &desc, state(),
                                                  nullptr, IID_PPV_ARGS(m_resource.GetAddressOf()));
        inkAssert(SUCCEEDED(hr), u"Failed to create ID3D12Resource for GpuBuffer: 0x{:X}.",
                  static_cast<std::uint32_t>(hr));

        m_gpuAddress = m_resource->GetGPUVirtualAddress();
    }

    { // Create byte address UAV.
        D3D12_UNORDERED_ACCESS_VIEW_DESC desc;
        desc.Format                      = DXGI_FORMAT_R32_TYPELESS;
        desc.ViewDimension               = D3D12_UAV_DIMENSION_BUFFER;
        desc.Buffer.FirstElement         = 0;
        desc.Buffer.NumElements          = static_cast<UINT>(m_size >> 2);
        desc.Buffer.StructureByteStride  = 0;
        desc.Buffer.CounterOffsetInBytes = 0;
        desc.Buffer.Flags                = D3D12_BUFFER_UAV_FLAG_RAW;

        m_byteAddressUAV.initUnorderedAccess(m_resource.Get(), nullptr, &desc);
    }
}

ink::GpuBuffer::GpuBuffer(GpuBuffer &&other) noexcept = default;

auto ink::GpuBuffer::operator=(GpuBuffer &&other) noexcept -> GpuBuffer & = default;

ink::GpuBuffer::~GpuBuffer() noexcept {}

ink::StructuredBuffer::StructuredBuffer() noexcept
    : GpuBuffer(), m_elementCount(), m_elementSize(), m_structuredBufferUAV() {}

ink::StructuredBuffer::StructuredBuffer(std::uint32_t count, std::uint32_t size) noexcept
    : GpuBuffer(std::size_t(count) * size),
      m_elementCount(count),
      m_elementSize(size),
      m_structuredBufferUAV() {
    if (this->size() == 0)
        return;

    // Create structured buffer UAV.
    D3D12_UNORDERED_ACCESS_VIEW_DESC desc;
    desc.Format                      = DXGI_FORMAT_UNKNOWN;
    desc.ViewDimension               = D3D12_UAV_DIMENSION_BUFFER;
    desc.Buffer.FirstElement         = 0;
    desc.Buffer.NumElements          = count;
    desc.Buffer.StructureByteStride  = size;
    desc.Buffer.CounterOffsetInBytes = 0;
    desc.Buffer.Flags                = D3D12_BUFFER_UAV_FLAG_NONE;

    m_structuredBufferUAV.initUnorderedAccess(m_resource.Get(), nullptr, &desc);
}

ink::StructuredBuffer::StructuredBuffer(StructuredBuffer &&other) noexcept = default;

auto ink::StructuredBuffer::operator=(StructuredBuffer &&other) noexcept
    -> StructuredBuffer & = default;

ink::StructuredBuffer::~StructuredBuffer() noexcept {}

auto ink::StructuredBuffer::reshape(std::uint32_t newCount, std::uint32_t newSize) noexcept
    -> void {
    std::size_t requiredSize = static_cast<std::size_t>(newCount) * newSize;

    m_elementCount = newCount;
    m_elementSize  = newSize;

    // This buffer could store the new elements and no need to recreate GPU buffer.
    if (requiredSize <= m_size) {
        // Recreate structured buffer UAV.
        D3D12_UNORDERED_ACCESS_VIEW_DESC desc;
        desc.Format                      = DXGI_FORMAT_UNKNOWN;
        desc.ViewDimension               = D3D12_UAV_DIMENSION_BUFFER;
        desc.Buffer.FirstElement         = 0;
        desc.Buffer.NumElements          = newCount;
        desc.Buffer.StructureByteStride  = newSize;
        desc.Buffer.CounterOffsetInBytes = 0;
        desc.Buffer.Flags                = D3D12_BUFFER_UAV_FLAG_NONE;

        m_structuredBufferUAV.initUnorderedAccess(m_resource.Get(), nullptr, &desc);
        return;
    }

    // Recreate the GPU buffer.
    m_resource.Reset();
    m_usageState = D3D12_RESOURCE_STATE_COMMON;

    // New buffer size.
    m_size = (requiredSize + 0xFF) & ~std::size_t(0xFF);

    auto &dev = RenderDevice::singleton();

    { // Create ID3D12Resource.
        [[maybe_unused]] HRESULT hr;

        const D3D12_HEAP_PROPERTIES heapProps{
            /* Type                 = */ D3D12_HEAP_TYPE_DEFAULT,
            /* CPUPageProperty      = */ D3D12_CPU_PAGE_PROPERTY_UNKNOWN,
            /* MemoryPoolPreference = */ D3D12_MEMORY_POOL_UNKNOWN,
            /* CreationNodeMask     = */ 0,
            /* VisibleNodeMask      = */ 0,
        };

        const D3D12_RESOURCE_DESC desc{
            /* Dimension        = */ D3D12_RESOURCE_DIMENSION_BUFFER,
            /* Alignment        = */ 0,
            /* Width            = */ m_size,
            /* Height           = */ 1,
            /* DepthOrArraySize = */ 1,
            /* MipLevels        = */ 1,
            /* Format           = */ DXGI_FORMAT_UNKNOWN,
            /* SampleDesc       = */
            {
                /* Count   = */ 1,
                /* Quality = */ 0,
            },
            /* Layout = */ D3D12_TEXTURE_LAYOUT_ROW_MAJOR,
            /* Flags  = */ D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS,
        };

        hr =
            dev.device()->CreateCommittedResource(&heapProps, D3D12_HEAP_FLAG_NONE, &desc, state(),
                                                  nullptr, IID_PPV_ARGS(m_resource.GetAddressOf()));
        inkAssert(SUCCEEDED(hr), u"Failed to create ID3D12Resource for StructuredBuffer: 0x{:X}.",
                  static_cast<std::uint32_t>(hr));

        m_gpuAddress = m_resource->GetGPUVirtualAddress();
    }

    { // Recreate byte address UAV.
        D3D12_UNORDERED_ACCESS_VIEW_DESC desc;
        desc.Format                      = DXGI_FORMAT_R32_TYPELESS;
        desc.ViewDimension               = D3D12_UAV_DIMENSION_BUFFER;
        desc.Buffer.FirstElement         = 0;
        desc.Buffer.NumElements          = static_cast<UINT>(m_size >> 2);
        desc.Buffer.StructureByteStride  = 0;
        desc.Buffer.CounterOffsetInBytes = 0;
        desc.Buffer.Flags                = D3D12_BUFFER_UAV_FLAG_RAW;

        m_byteAddressUAV.initUnorderedAccess(m_resource.Get(), nullptr, &desc);
    }

    { // Create structured buffer UAV.
        D3D12_UNORDERED_ACCESS_VIEW_DESC desc;
        desc.Format                      = DXGI_FORMAT_UNKNOWN;
        desc.ViewDimension               = D3D12_UAV_DIMENSION_BUFFER;
        desc.Buffer.FirstElement         = 0;
        desc.Buffer.NumElements          = newCount;
        desc.Buffer.StructureByteStride  = newSize;
        desc.Buffer.CounterOffsetInBytes = 0;
        desc.Buffer.Flags                = D3D12_BUFFER_UAV_FLAG_NONE;

        m_structuredBufferUAV.initUnorderedAccess(m_resource.Get(), nullptr, &desc);
    }
}

ink::PixelBuffer::PixelBuffer() noexcept
    : GpuResource(),
      m_width(),
      m_height(),
      m_arraySize(),
      m_sampleCount(),
      m_mipLevels(),
      m_pixelFormat(DXGI_FORMAT_UNKNOWN) {}

ink::PixelBuffer::PixelBuffer(PixelBuffer &&other) noexcept = default;

auto ink::PixelBuffer::operator=(PixelBuffer &&other) noexcept -> PixelBuffer & = default;

ink::PixelBuffer::~PixelBuffer() noexcept {}

namespace {

/// @brief
///   Calculate maximum supported mipmap levels for the specified image width.
///
/// @param width
///   Width in pixel of the image to get maximum supported mipmap levels.
///
/// @return
///   Maximum supported mipmap levels.
[[nodiscard]]
__forceinline auto maxMipLevels(std::uint32_t width) noexcept -> std::uint32_t {
    std::uint32_t result = 0;
    while (width) {
        width >>= 1;
        result += 1;
    }

    return result;
}

} // namespace

ink::ColorBuffer::ColorBuffer() noexcept
    : PixelBuffer(), m_clearColor(), m_rtv(), m_srv(), m_uav() {}

ink::ColorBuffer::ColorBuffer(std::uint32_t width,
                              std::uint32_t height,
                              std::uint32_t arraySize,
                              DXGI_FORMAT   format,
                              std::uint32_t mipLevels,
                              std::uint32_t sampleCount) noexcept
    : PixelBuffer(), m_clearColor(), m_rtv(), m_srv(), m_uav() {
    // Clamp mipmap levels.
    const std::uint32_t maxMip = maxMipLevels(width | height);
    if (mipLevels == 0 || mipLevels > maxMip)
        mipLevels = maxMip;

    if (sampleCount == 0)
        sampleCount = 1;

    m_width       = width;
    m_height      = height;
    m_arraySize   = arraySize;
    m_sampleCount = sampleCount;
    m_mipLevels   = mipLevels;
    m_pixelFormat = format;

    auto &dev = RenderDevice::singleton();

    // Checks if this color buffer supports unordered access.
    const bool supportUAV = (sampleCount == 1) && dev.supportUnorderedAccess(format);

    { // Create ID3D12Resource.
        [[maybe_unused]] HRESULT hr;

        const D3D12_HEAP_PROPERTIES heapProps{
            /* Type                 = */ D3D12_HEAP_TYPE_DEFAULT,
            /* CPUPageProperty      = */ D3D12_CPU_PAGE_PROPERTY_UNKNOWN,
            /* MemoryPoolPreference = */ D3D12_MEMORY_POOL_UNKNOWN,
            /* CreationNodeMask     = */ 0,
            /* VisibleNodeMask      = */ 0,
        };

        D3D12_RESOURCE_FLAGS flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;
        if (supportUAV)
            flags |= D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;

        const D3D12_RESOURCE_DESC desc{
            /* Dimension        = */ D3D12_RESOURCE_DIMENSION_TEXTURE2D,
            /* Alignment        = */ 0,
            /* Width            = */ width,
            /* Height           = */ height,
            /* DepthOrArraySize = */ static_cast<UINT16>(arraySize),
            /* MipLevels        = */ static_cast<UINT16>(mipLevels),
            /* Format           = */ format,
            /* SampleDesc       = */
            {
                /* Count   = */ sampleCount,
                /* Quality = */ 0,
            },
            /* Layout = */ D3D12_TEXTURE_LAYOUT_UNKNOWN,
            /* Flags  = */ flags,
        };

        hr =
            dev.device()->CreateCommittedResource(&heapProps, D3D12_HEAP_FLAG_NONE, &desc, state(),
                                                  nullptr, IID_PPV_ARGS(m_resource.GetAddressOf()));
        inkAssert(SUCCEEDED(hr), u"Failed to create ID3D12Resource for ColorBuffer: 0x{:X}.",
                  static_cast<std::uint32_t>(hr));
    }

    { // Create render target view.
        D3D12_RENDER_TARGET_VIEW_DESC desc;
        desc.Format = format;

        if (arraySize > 1 && sampleCount > 1) {
            desc.ViewDimension                    = D3D12_RTV_DIMENSION_TEXTURE2DMSARRAY;
            desc.Texture2DMSArray.FirstArraySlice = 0;
            desc.Texture2DMSArray.ArraySize       = arraySize;
        } else if (arraySize > 1) {
            desc.ViewDimension                  = D3D12_RTV_DIMENSION_TEXTURE2DARRAY;
            desc.Texture2DArray.MipSlice        = 0;
            desc.Texture2DArray.FirstArraySlice = 0;
            desc.Texture2DArray.ArraySize       = arraySize;
            desc.Texture2DArray.PlaneSlice      = 0;
        } else if (sampleCount > 1) {
            desc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2DMS;
        } else {
            desc.ViewDimension        = D3D12_RTV_DIMENSION_TEXTURE2D;
            desc.Texture2D.MipSlice   = 0;
            desc.Texture2D.PlaneSlice = 0;
        }

        m_rtv.initRenderTarget(m_resource.Get(), &desc);
    }

    { // Create shader resource view.
        D3D12_SHADER_RESOURCE_VIEW_DESC desc;
        desc.Format                  = format;
        desc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;

        if (arraySize > 1 && sampleCount > 1) {
            desc.ViewDimension                    = D3D12_SRV_DIMENSION_TEXTURE2DMSARRAY;
            desc.Texture2DMSArray.FirstArraySlice = 0;
            desc.Texture2DMSArray.ArraySize       = arraySize;
        } else if (arraySize > 1) {
            desc.ViewDimension                      = D3D12_SRV_DIMENSION_TEXTURE2DARRAY;
            desc.Texture2DArray.MostDetailedMip     = 0;
            desc.Texture2DArray.MipLevels           = mipLevels;
            desc.Texture2DArray.FirstArraySlice     = 0;
            desc.Texture2DArray.ArraySize           = arraySize;
            desc.Texture2DArray.PlaneSlice          = 0;
            desc.Texture2DArray.ResourceMinLODClamp = 0;
        } else if (sampleCount > 1) {
            desc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2DMS;
        } else {
            desc.ViewDimension                 = D3D12_SRV_DIMENSION_TEXTURE2D;
            desc.Texture2D.MostDetailedMip     = 0;
            desc.Texture2D.MipLevels           = mipLevels;
            desc.Texture2D.PlaneSlice          = 0;
            desc.Texture2D.ResourceMinLODClamp = 0;
        }

        m_srv.initShaderResource(m_resource.Get(), &desc);
    }

    // Create unordered access view.
    if (supportUAV) {
        D3D12_UNORDERED_ACCESS_VIEW_DESC desc;
        desc.Format = format;

        if (arraySize > 1) {
            desc.ViewDimension                  = D3D12_UAV_DIMENSION_TEXTURE2DARRAY;
            desc.Texture2DArray.MipSlice        = 0;
            desc.Texture2DArray.FirstArraySlice = 0;
            desc.Texture2DArray.ArraySize       = arraySize;
            desc.Texture2DArray.PlaneSlice      = 0;
        } else {
            desc.ViewDimension        = D3D12_UAV_DIMENSION_TEXTURE2D;
            desc.Texture2D.MipSlice   = 0;
            desc.Texture2D.PlaneSlice = 0;
        }

        m_uav.initUnorderedAccess(m_resource.Get(), nullptr, &desc);
    }
}

ink::ColorBuffer::ColorBuffer(ColorBuffer &&other) noexcept = default;

auto ink::ColorBuffer::operator=(ColorBuffer &&other) noexcept -> ColorBuffer & = default;

ink::ColorBuffer::~ColorBuffer() noexcept {}

auto ink::ColorBuffer::resetSwapChainBuffer(Microsoft::WRL::ComPtr<ID3D12Resource> buffer) noexcept
    -> void {
    const D3D12_RESOURCE_DESC desc = buffer->GetDesc();

    m_resource    = std::move(buffer);
    m_usageState  = D3D12_RESOURCE_STATE_PRESENT;
    m_width       = static_cast<std::uint32_t>(desc.Width);
    m_height      = static_cast<std::uint32_t>(desc.Height);
    m_arraySize   = desc.DepthOrArraySize;
    m_sampleCount = desc.SampleDesc.Count;
    m_mipLevels   = desc.MipLevels;
    m_pixelFormat = desc.Format;

    m_rtv.initRenderTarget(m_resource.Get(), nullptr);
}

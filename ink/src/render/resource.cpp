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

ink::ReadbackBuffer::ReadbackBuffer() noexcept : GpuResource(), m_size(), m_gpuAddress() {}

ink::ReadbackBuffer::ReadbackBuffer(std::size_t size) noexcept
    : GpuResource(), m_size((size + 0xFF) & ~std::size_t(0xFF)), m_gpuAddress() {
    [[maybe_unused]] HRESULT hr;

    auto &dev = RenderDevice::singleton();

    // Create ID3D12Resource.
    const D3D12_HEAP_PROPERTIES heapProps{
        /* Type                 = */ D3D12_HEAP_TYPE_READBACK,
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
        /* Flags  = */ D3D12_RESOURCE_FLAG_NONE,
    };

    hr = dev.device()->CreateCommittedResource(&heapProps, D3D12_HEAP_FLAG_NONE, &desc,
                                               D3D12_RESOURCE_STATE_COPY_DEST, nullptr,
                                               IID_PPV_ARGS(m_resource.GetAddressOf()));

    inkAssert(SUCCEEDED(hr), u"Failed to create ID3D12Resource for readback buffer: 0x{:X}.",
              static_cast<std::uint32_t>(hr));

    m_usageState = D3D12_RESOURCE_STATE_COPY_DEST;
    m_gpuAddress = m_resource->GetGPUVirtualAddress();
}

ink::ReadbackBuffer::ReadbackBuffer(ReadbackBuffer &&other) noexcept = default;

auto ink::ReadbackBuffer::operator=(ReadbackBuffer &&other) noexcept -> ReadbackBuffer & = default;

ink::ReadbackBuffer::~ReadbackBuffer() noexcept {}

auto ink::ReadbackBuffer::unmap() noexcept -> void {
    D3D12_RANGE range{};
    m_resource->Unmap(0, &range);
}

auto ink::ReadbackBuffer::map() const noexcept -> const void * {
    [[maybe_unused]] HRESULT hr;

    void       *ptr;
    D3D12_RANGE range{0, m_size};

    hr = m_resource->Map(0, &range, &ptr);
    inkAssert(SUCCEEDED(hr), u"Failed to map readback buffer: 0x{:X}.",
              static_cast<std::uint32_t>(hr));

    return ptr;
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

namespace {

/// @brief
///   Get depth pixel format from a depth stencil format.
///
/// @param format
///   The depth stencil pixel format.
///
/// @return
///   Depth pixel format from the depth stencil format.
[[nodiscard]]
auto getDepthFormat(DXGI_FORMAT format) noexcept -> DXGI_FORMAT {
    switch (format) {
    case DXGI_FORMAT_R32G8X24_TYPELESS:
    case DXGI_FORMAT_D32_FLOAT_S8X24_UINT:
    case DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS:
    case DXGI_FORMAT_X32_TYPELESS_G8X24_UINT:
        return DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS;

    case DXGI_FORMAT_R32_TYPELESS:
    case DXGI_FORMAT_D32_FLOAT:
    case DXGI_FORMAT_R32_FLOAT:
        return DXGI_FORMAT_R32_FLOAT;

    case DXGI_FORMAT_R24G8_TYPELESS:
    case DXGI_FORMAT_D24_UNORM_S8_UINT:
    case DXGI_FORMAT_R24_UNORM_X8_TYPELESS:
    case DXGI_FORMAT_X24_TYPELESS_G8_UINT:
        return DXGI_FORMAT_R24_UNORM_X8_TYPELESS;

    case DXGI_FORMAT_R16_TYPELESS:
    case DXGI_FORMAT_D16_UNORM:
    case DXGI_FORMAT_R16_UNORM:
        return DXGI_FORMAT_R16_UNORM;

    default:
        return DXGI_FORMAT_UNKNOWN;
    }
}

} // namespace

ink::DepthBuffer::DepthBuffer() noexcept
    : PixelBuffer(),
      m_clearDepth(1.0f),
      m_clearStencil(),
      m_dsv(),
      m_depthReadOnlyView(),
      m_depthSRV() {}

ink::DepthBuffer::DepthBuffer(std::uint32_t width,
                              std::uint32_t height,
                              DXGI_FORMAT   format,
                              std::uint32_t sampleCount) noexcept
    : PixelBuffer(),
      m_clearDepth(1.0f),
      m_clearStencil(),
      m_dsv(),
      m_depthReadOnlyView(),
      m_depthSRV() {
    if (sampleCount == 0)
        sampleCount = 1;

    this->m_width       = width;
    this->m_height      = height;
    this->m_arraySize   = 1;
    this->m_sampleCount = sampleCount;
    this->m_mipLevels   = 1;
    this->m_pixelFormat = format;

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
            /* Dimension        = */ D3D12_RESOURCE_DIMENSION_TEXTURE2D,
            /* Alignment        = */ 0,
            /* Width            = */ width,
            /* Height           = */ height,
            /* DepthOrArraySize = */ 1,
            /* MipLevels        = */ 1,
            /* Format           = */ format,
            /* SampleDesc       = */
            {
                /* Count   = */ sampleCount,
                /* Quality = */ 0,
            },
            /* Layout = */ D3D12_TEXTURE_LAYOUT_UNKNOWN,
            /* Flags  = */ D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL,
        };

        D3D12_CLEAR_VALUE clearValue;
        clearValue.Format               = m_pixelFormat;
        clearValue.DepthStencil.Depth   = 1.0f;
        clearValue.DepthStencil.Stencil = 0;

        hr = dev.device()->CreateCommittedResource(&heapProps, D3D12_HEAP_FLAG_NONE, &desc, state(),
                                                   &clearValue,
                                                   IID_PPV_ARGS(m_resource.GetAddressOf()));
        inkAssert(SUCCEEDED(hr), u"Failed to create ID3D12Resource for depth buffer: 0x{:X}.",
                  static_cast<std::uint32_t>(hr));
    }

    { // Create depth stencil view.
        D3D12_DEPTH_STENCIL_VIEW_DESC desc;
        desc.Format = format;
        desc.Flags  = D3D12_DSV_FLAG_NONE;

        if (sampleCount > 1) {
            desc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2DMS;
        } else {
            desc.ViewDimension      = D3D12_DSV_DIMENSION_TEXTURE2D;
            desc.Texture2D.MipSlice = 0;
        }

        m_dsv.initDepthStencil(m_resource.Get(), &desc);
    }

    { // Create depth read-only view.
        D3D12_DEPTH_STENCIL_VIEW_DESC desc;
        desc.Format = format;
        desc.Flags  = D3D12_DSV_FLAG_READ_ONLY_DEPTH;

        if (sampleCount > 1) {
            desc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2DMS;
        } else {
            desc.ViewDimension      = D3D12_DSV_DIMENSION_TEXTURE2D;
            desc.Texture2D.MipSlice = 0;
        }

        m_depthReadOnlyView.initDepthStencil(m_resource.Get(), &desc);
    }

    const DXGI_FORMAT depthFormat = getDepthFormat(format);
    if (depthFormat != DXGI_FORMAT_UNKNOWN) {
        D3D12_SHADER_RESOURCE_VIEW_DESC desc;
        desc.Format                  = depthFormat;
        desc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;

        if (sampleCount > 1) {
            desc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2DMS;
        } else {
            desc.ViewDimension                 = D3D12_SRV_DIMENSION_TEXTURE2D;
            desc.Texture2D.MostDetailedMip     = 0;
            desc.Texture2D.MipLevels           = 1;
            desc.Texture2D.PlaneSlice          = 0;
            desc.Texture2D.ResourceMinLODClamp = 0.0f;
        }

        m_depthSRV.initShaderResource(m_resource.Get(), &desc);
    }
}

ink::DepthBuffer::DepthBuffer(DepthBuffer &&other) noexcept = default;

auto ink::DepthBuffer::operator=(DepthBuffer &&other) noexcept -> DepthBuffer & = default;

ink::DepthBuffer::~DepthBuffer() noexcept {}

ink::Texture2D::Texture2D() noexcept : PixelBuffer(), m_isCube(false), m_srv() {}

ink::Texture2D::Texture2D(std::uint32_t width,
                          std::uint32_t height,
                          std::uint32_t arraySize,
                          DXGI_FORMAT   format,
                          std::uint32_t mipLevels,
                          bool          isCube) noexcept
    : PixelBuffer(), m_isCube(false), m_srv() {
    // Check cube texture support.
    isCube   = isCube && (arraySize % 6 == 0);
    m_isCube = isCube;

    // Maximum mipmap levels.
    const std::uint32_t maxMips = maxMipLevels(width | height);
    if (mipLevels == 0 || mipLevels > maxMips)
        mipLevels = maxMips;

    this->m_width       = width;
    this->m_height      = height;
    this->m_arraySize   = arraySize;
    this->m_sampleCount = 1;
    this->m_mipLevels   = mipLevels;
    this->m_pixelFormat = format;

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
            /* Dimension        = */ D3D12_RESOURCE_DIMENSION_TEXTURE2D,
            /* Alignment        = */ 0,
            /* Width            = */ width,
            /* Height           = */ height,
            /* DepthOrArraySize = */ static_cast<UINT16>(arraySize),
            /* MipLevels        = */ static_cast<UINT16>(mipLevels),
            /* Format           = */ format,
            /* SampleDesc       = */
            {
                /* Count   = */ 1,
                /* Quality = */ 0,
            },
            /* Layout = */ D3D12_TEXTURE_LAYOUT_UNKNOWN,
            /* Flags  = */ D3D12_RESOURCE_FLAG_NONE,
        };

        hr =
            dev.device()->CreateCommittedResource(&heapProps, D3D12_HEAP_FLAG_NONE, &desc, state(),
                                                  nullptr, IID_PPV_ARGS(m_resource.GetAddressOf()));
        inkAssert(SUCCEEDED(hr), u"Failed to create ID3D12Resource for Texture2D: 0x{:X}.",
                  static_cast<std::uint32_t>(hr));
    }

    // Create shader resource view.
    if (isCube) {
        D3D12_SHADER_RESOURCE_VIEW_DESC desc;
        desc.Format                  = format;
        desc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
        if (arraySize > 6) {
            desc.ViewDimension                        = D3D12_SRV_DIMENSION_TEXTURECUBEARRAY;
            desc.TextureCubeArray.MostDetailedMip     = 0;
            desc.TextureCubeArray.MipLevels           = mipLevels;
            desc.TextureCubeArray.First2DArrayFace    = 0;
            desc.TextureCubeArray.NumCubes            = arraySize / 6;
            desc.TextureCubeArray.ResourceMinLODClamp = 0.0f;
        } else {
            desc.ViewDimension                   = D3D12_SRV_DIMENSION_TEXTURECUBE;
            desc.TextureCube.MostDetailedMip     = 0;
            desc.TextureCube.MipLevels           = mipLevels;
            desc.TextureCube.ResourceMinLODClamp = 0.0f;
        }

        m_srv.initShaderResource(m_resource.Get(), &desc);
    } else {
        m_srv.initShaderResource(m_resource.Get(), nullptr);
    }
}

ink::Texture2D::Texture2D(Texture2D &&other) noexcept = default;

auto ink::Texture2D::operator=(Texture2D &&other) noexcept -> Texture2D & = default;

ink::Texture2D::~Texture2D() noexcept {}

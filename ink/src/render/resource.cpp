#include "ink/render/resource.hpp"
#include "ink/core/exception.hpp"
#include "ink/render/device.hpp"

using namespace ink;

ink::GpuResource::GpuResource() noexcept
    : m_resource(), m_usageState(D3D12_RESOURCE_STATE_COMMON) {}

ink::GpuResource::GpuResource(GpuResource &&other) noexcept = default;

ink::GpuResource::~GpuResource() noexcept = default;

auto ink::GpuResource::operator=(GpuResource &&other) noexcept -> GpuResource & = default;

ink::GpuBuffer::GpuBuffer(RenderDevice &renderDevice, ID3D12Device *device, std::size_t size)
    : GpuResource(),
      m_size((size + 0xFF) & ~std::size_t(0xFF)),
      m_gpuAddress(),
      m_byteAddressUAV(renderDevice.newUnorderedAccessView()) {
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

        HRESULT hr = device->CreateCommittedResource(&heapProps, D3D12_HEAP_FLAG_NONE, &desc,
                                                     D3D12_RESOURCE_STATE_COMMON, nullptr,
                                                     IID_PPV_ARGS(m_resource.GetAddressOf()));
        if (FAILED(hr))
            throw RenderAPIException(hr, "Failed to create ID3D12Resource for GpuBuffer.");

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

        m_byteAddressUAV.update(m_resource.Get(), nullptr, &desc);
    }
}

ink::GpuBuffer::GpuBuffer() noexcept
    : GpuResource(), m_size(), m_gpuAddress(), m_byteAddressUAV() {}

ink::GpuBuffer::GpuBuffer(GpuBuffer &&other) noexcept = default;

ink::GpuBuffer::~GpuBuffer() noexcept = default;

auto ink::GpuBuffer::operator=(GpuBuffer &&other) noexcept -> GpuBuffer & = default;

ink::StructuredBuffer::StructuredBuffer(RenderDevice &renderDevice,
                                        ID3D12Device *device,
                                        std::uint32_t elementCount,
                                        std::uint32_t elementSize)
    : GpuBuffer(renderDevice, device, static_cast<std::size_t>(elementCount) * elementSize),
      m_device(device),
      m_elementCount(elementCount),
      m_elementSize(elementSize),
      m_structuredBufferUAV(renderDevice.newUnorderedAccessView()) {
    // Create structured buffer UAV.
    D3D12_UNORDERED_ACCESS_VIEW_DESC desc;
    desc.Format                      = DXGI_FORMAT_UNKNOWN;
    desc.ViewDimension               = D3D12_UAV_DIMENSION_BUFFER;
    desc.Buffer.FirstElement         = 0;
    desc.Buffer.NumElements          = m_elementCount;
    desc.Buffer.StructureByteStride  = m_elementSize;
    desc.Buffer.CounterOffsetInBytes = 0;
    desc.Buffer.Flags                = D3D12_BUFFER_UAV_FLAG_NONE;

    m_structuredBufferUAV.update(m_resource.Get(), nullptr, &desc);
}

ink::StructuredBuffer::StructuredBuffer() noexcept
    : GpuBuffer(), m_device(), m_elementCount(), m_elementSize(), m_structuredBufferUAV() {}

ink::StructuredBuffer::StructuredBuffer(StructuredBuffer &&other) noexcept = default;

ink::StructuredBuffer::~StructuredBuffer() noexcept = default;

auto ink::StructuredBuffer::operator=(StructuredBuffer &&other) noexcept
    -> StructuredBuffer & = default;

auto ink::StructuredBuffer::reshape(std::uint32_t newCount, std::uint32_t newSize) -> void {
    std::size_t requiredSize = static_cast<std::size_t>(newCount) * newSize;

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

        m_structuredBufferUAV.update(m_resource.Get(), nullptr, &desc);

        m_elementCount = newCount;
        m_elementSize  = newSize;

        return;
    }

    { // Create ID3D12Resource.
        Microsoft::WRL::ComPtr<ID3D12Resource> newResource;
        requiredSize = (requiredSize + 0xFF) & ~std::size_t(0xFF);

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
            /* Width            = */ static_cast<UINT64>(requiredSize),
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

        HRESULT hr = m_device->CreateCommittedResource(&heapProps, D3D12_HEAP_FLAG_NONE, &desc,
                                                       D3D12_RESOURCE_STATE_COMMON, nullptr,
                                                       IID_PPV_ARGS(newResource.GetAddressOf()));
        if (FAILED(hr))
            throw RenderAPIException(hr, "Failed to recreate ID3D12Resource for StructuredBuffer.");

        m_resource     = std::move(newResource);
        m_usageState   = D3D12_RESOURCE_STATE_COMMON;
        m_size         = requiredSize;
        m_gpuAddress   = m_resource->GetGPUVirtualAddress();
        m_elementCount = newCount;
        m_elementSize  = newSize;
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

        m_byteAddressUAV.update(m_resource.Get(), nullptr, &desc);
    }

    { // Recreate structured buffer UAV.
        D3D12_UNORDERED_ACCESS_VIEW_DESC desc;
        desc.Format                      = DXGI_FORMAT_UNKNOWN;
        desc.ViewDimension               = D3D12_UAV_DIMENSION_BUFFER;
        desc.Buffer.FirstElement         = 0;
        desc.Buffer.NumElements          = newCount;
        desc.Buffer.StructureByteStride  = newSize;
        desc.Buffer.CounterOffsetInBytes = 0;
        desc.Buffer.Flags                = D3D12_BUFFER_UAV_FLAG_NONE;

        m_structuredBufferUAV.update(m_resource.Get(), nullptr, &desc);
    }
}

ink::PixelBuffer::PixelBuffer() noexcept
    : GpuResource(),
      m_width(),
      m_height(),
      m_arraySize(),
      m_sampleCount(),
      m_mipLevels(),
      m_pixelFormat(),
      m_shaderResourceView(),
      m_unorderedAccessView() {}

ink::PixelBuffer::PixelBuffer(PixelBuffer &&other) noexcept = default;

ink::PixelBuffer::~PixelBuffer() noexcept = default;

auto ink::PixelBuffer::operator=(PixelBuffer &&other) noexcept -> PixelBuffer & = default;

namespace {

/// @brief
///   Calculate maximum supported mipmap levels for the specified image width.
///
/// @param width
///   Width in pixel of the image to get maximum supported mipmap levels.
///
/// @return
///   Maximum supported mipmap levels.
[[nodiscard]] auto maxMipLevels(std::uint32_t width) noexcept -> std::uint32_t {
    std::uint32_t result = 0;
    while (width) {
        width >>= 1;
        result += 1;
    }

    return result;
}

} // namespace

ink::ColorBuffer::ColorBuffer(RenderDevice &renderDevice,
                              ID3D12Device *device,
                              std::uint32_t width,
                              std::uint32_t height,
                              std::uint32_t arraySize,
                              DXGI_FORMAT   format,
                              std::uint32_t mipLevels,
                              std::uint32_t sampleCount)
    : PixelBuffer(), m_clearColor(), m_renderTargetView() {
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

    // Checks if this color buffer supports unordered access.
    const bool supportUAV = (sampleCount == 1) && renderDevice.supportUnorderedAccess(format);

    { // Create ID3D12Resource.
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

        HRESULT hr = device->CreateCommittedResource(&heapProps, D3D12_HEAP_FLAG_NONE, &desc,
                                                     D3D12_RESOURCE_STATE_COMMON, nullptr,
                                                     IID_PPV_ARGS(m_resource.GetAddressOf()));
        if (FAILED(hr))
            throw RenderAPIException(hr, "Failed to create ID3D12Resource for ColorBuffer.");
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

        m_renderTargetView = renderDevice.newRenderTargetView();
        m_renderTargetView.update(m_resource.Get(), &desc);
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

        m_shaderResourceView = renderDevice.newShaderResourceView();
        m_shaderResourceView.update(m_resource.Get(), &desc);
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

        m_unorderedAccessView = renderDevice.newUnorderedAccessView();
        m_unorderedAccessView.update(m_resource.Get(), nullptr, &desc);
    }
}

ink::ColorBuffer::ColorBuffer() noexcept : PixelBuffer(), m_clearColor(), m_renderTargetView() {}

ink::ColorBuffer::ColorBuffer(ColorBuffer &&other) noexcept = default;

ink::ColorBuffer::~ColorBuffer() noexcept = default;

auto ink::ColorBuffer::operator=(ColorBuffer &&other) noexcept -> ColorBuffer & = default;

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

    m_renderTargetView.update(m_resource.Get(), nullptr);
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
[[nodiscard]] auto getDepthFormat(DXGI_FORMAT format) noexcept -> DXGI_FORMAT {
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

ink::DepthBuffer::DepthBuffer(RenderDevice &renderDevice,
                              ID3D12Device *device,
                              std::uint32_t width,
                              std::uint32_t height,
                              DXGI_FORMAT   format,
                              std::uint32_t sampleCount)
    : PixelBuffer(),
      m_clearDepth(1.0f),
      m_clearStencil(),
      m_depthStencilView(),
      m_depthReadOnlyView() {
    if (sampleCount == 0)
        sampleCount = 1;

    this->m_width       = width;
    this->m_height      = height;
    this->m_arraySize   = 1;
    this->m_sampleCount = sampleCount;
    this->m_mipLevels   = 1;
    this->m_pixelFormat = format;

    const DXGI_FORMAT depthFormat = getDepthFormat(format);
    const bool supportUAV = (sampleCount == 1) && renderDevice.supportUnorderedAccess(depthFormat);

    { // Create ID3D12Resource.
        const D3D12_HEAP_PROPERTIES heapProps{
            /* Type                 = */ D3D12_HEAP_TYPE_DEFAULT,
            /* CPUPageProperty      = */ D3D12_CPU_PAGE_PROPERTY_UNKNOWN,
            /* MemoryPoolPreference = */ D3D12_MEMORY_POOL_UNKNOWN,
            /* CreationNodeMask     = */ 0,
            /* VisibleNodeMask      = */ 0,
        };

        D3D12_RESOURCE_FLAGS flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;
        if (supportUAV)
            flags |= D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;

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
            /* Flags  = */ flags,
        };

        D3D12_CLEAR_VALUE clearValue;
        clearValue.Format               = m_pixelFormat;
        clearValue.DepthStencil.Depth   = 1.0f;
        clearValue.DepthStencil.Stencil = 0;

        HRESULT hr = device->CreateCommittedResource(&heapProps, D3D12_HEAP_FLAG_NONE, &desc,
                                                     D3D12_RESOURCE_STATE_COMMON, &clearValue,
                                                     IID_PPV_ARGS(m_resource.GetAddressOf()));
        if (FAILED(hr))
            throw RenderAPIException(hr, "Failed to create ID3D12Resource for DepthBuffer.");
    }

    { // Create depth stencil view.
        m_depthStencilView = renderDevice.newDepthStencilView();

        D3D12_DEPTH_STENCIL_VIEW_DESC desc;
        desc.Format = format;
        desc.Flags  = D3D12_DSV_FLAG_NONE;

        if (sampleCount > 1) {
            desc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2DMS;
        } else {
            desc.ViewDimension      = D3D12_DSV_DIMENSION_TEXTURE2D;
            desc.Texture2D.MipSlice = 0;
        }

        m_depthStencilView.update(m_resource.Get(), &desc);
    }

    { // Create depth read-only view.
        m_depthReadOnlyView = renderDevice.newDepthStencilView();

        D3D12_DEPTH_STENCIL_VIEW_DESC desc;
        desc.Format = format;
        desc.Flags  = D3D12_DSV_FLAG_READ_ONLY_DEPTH;

        if (sampleCount > 1) {
            desc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2DMS;
        } else {
            desc.ViewDimension      = D3D12_DSV_DIMENSION_TEXTURE2D;
            desc.Texture2D.MipSlice = 0;
        }

        m_depthReadOnlyView.update(m_resource.Get(), &desc);
    }

    // Create depth shader-resource view.
    if (depthFormat != DXGI_FORMAT_UNKNOWN) {
        m_shaderResourceView = renderDevice.newShaderResourceView();

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

        m_shaderResourceView.update(m_resource.Get(), &desc);
    }

    // Create unordered access view.
    if (supportUAV) {
        m_unorderedAccessView = renderDevice.newUnorderedAccessView();

        D3D12_UNORDERED_ACCESS_VIEW_DESC desc;
        desc.Format               = depthFormat;
        desc.ViewDimension        = D3D12_UAV_DIMENSION_TEXTURE2D;
        desc.Texture2D.MipSlice   = 0;
        desc.Texture2D.PlaneSlice = 0;

        m_unorderedAccessView.update(m_resource.Get(), nullptr, &desc);
    }
}

ink::DepthBuffer::DepthBuffer() noexcept
    : PixelBuffer(),
      m_clearDepth(1.0f),
      m_clearStencil(),
      m_depthStencilView(),
      m_depthReadOnlyView() {}

ink::DepthBuffer::DepthBuffer(DepthBuffer &&other) noexcept = default;

ink::DepthBuffer::~DepthBuffer() noexcept = default;

auto ink::DepthBuffer::operator=(DepthBuffer &&other) noexcept -> DepthBuffer & = default;

ink::Texture2D::Texture2D(RenderDevice &renderDevice,
                          ID3D12Device *device,
                          std::uint32_t width,
                          std::uint32_t height,
                          std::uint32_t arraySize,
                          DXGI_FORMAT   format,
                          std::uint32_t mipLevels,
                          bool          isCube)
    : PixelBuffer(), m_isCube() {
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

    // Check unordered access support.
    const bool supportUAV = renderDevice.supportUnorderedAccess(format);

    { // Create ID3D12Resource.
        const D3D12_HEAP_PROPERTIES heapProps{
            /* Type                 = */ D3D12_HEAP_TYPE_DEFAULT,
            /* CPUPageProperty      = */ D3D12_CPU_PAGE_PROPERTY_UNKNOWN,
            /* MemoryPoolPreference = */ D3D12_MEMORY_POOL_UNKNOWN,
            /* CreationNodeMask     = */ 0,
            /* VisibleNodeMask      = */ 0,
        };

        D3D12_RESOURCE_FLAGS flags = D3D12_RESOURCE_FLAG_NONE;
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
                /* Count   = */ 1,
                /* Quality = */ 0,
            },
            /* Layout = */ D3D12_TEXTURE_LAYOUT_UNKNOWN,
            /* Flags  = */ flags,
        };

        HRESULT hr = device->CreateCommittedResource(&heapProps, D3D12_HEAP_FLAG_NONE, &desc,
                                                     D3D12_RESOURCE_STATE_COMMON, nullptr,
                                                     IID_PPV_ARGS(m_resource.GetAddressOf()));
        if (FAILED(hr))
            throw RenderAPIException(hr, "Failed to create ID3D12Resource for Texture2D.");
    }

    // Create shader resource view.
    if (isCube) {
        m_shaderResourceView = renderDevice.newShaderResourceView();

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

        m_shaderResourceView.update(m_resource.Get(), &desc);
    } else {
        m_shaderResourceView = renderDevice.newShaderResourceView();
        m_shaderResourceView.update(m_resource.Get(), nullptr);
    }

    // Create unordered access view.
    if (supportUAV) {
        m_unorderedAccessView = renderDevice.newUnorderedAccessView();

        D3D12_UNORDERED_ACCESS_VIEW_DESC desc;
        desc.Format               = format;
        desc.ViewDimension        = D3D12_UAV_DIMENSION_TEXTURE2D;
        desc.Texture2D.MipSlice   = 0;
        desc.Texture2D.PlaneSlice = 0;

        m_unorderedAccessView.update(m_resource.Get(), nullptr, &desc);
    }
}

ink::Texture2D::Texture2D() noexcept : PixelBuffer(), m_isCube() {}

ink::Texture2D::Texture2D(Texture2D &&other) noexcept = default;

ink::Texture2D::~Texture2D() noexcept = default;

auto ink::Texture2D::operator=(Texture2D &&other) noexcept -> Texture2D & = default;

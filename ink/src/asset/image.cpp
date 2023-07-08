#include "ink/asset/image.h"
#include "ink/core/assert.h"

#include <wincodec.h>
#include <wrl/client.h>

using namespace ink;
using Microsoft::WRL::ComPtr;

namespace {

class ImageFactory {
private:
    /// @brief
    ///   Create a WIC image factory instance.
    ImageFactory() noexcept;

    /// @brief
    ///   Destroy this image factory instance.
    ~ImageFactory() noexcept;

public:
    /// @brief
    ///   Get WIC image factory instance.
    ///
    /// @return
    ///   WIC image factory instance.
    [[nodiscard]]
    static auto get() noexcept -> IWICImagingFactory *;

private:
    /// @brief
    ///   WIC image factory instance.
    ComPtr<IWICImagingFactory> m_factory;
};

ImageFactory::ImageFactory() noexcept {
    [[maybe_unused]] HRESULT hr;

    // initialize COM.
    hr = CoInitializeEx(nullptr, COINIT_MULTITHREADED);
    inkAssert(SUCCEEDED(hr) || (hr == RPC_E_CHANGED_MODE), u"Failed to initialize COM: 0x{:X}.",
              static_cast<std::uint32_t>(hr));

    // Create WIC imaging factory.
    hr = CoCreateInstance(CLSID_WICImagingFactory2, nullptr, CLSCTX_INPROC_SERVER,
                          IID_PPV_ARGS(m_factory.GetAddressOf()));
    inkAssert(SUCCEEDED(hr), u"Failed to create WICImagingFactory: 0x{:X}.",
              static_cast<std::uint32_t>(hr));
}

ImageFactory::~ImageFactory() noexcept {
    m_factory.Reset();
    CoUninitialize();
}

auto ImageFactory::get() noexcept -> IWICImagingFactory * {
    static ImageFactory instance;
    return instance.m_factory.Get();
}

/// @brief
///   Convert WIC pixel format to DXGI pixel format.
///
/// @param guid
///   WIC pixel format.
///
/// @return
///   The corresponding DXGI pixel format. Return @p DXGI_FORMAT_UNKNOWN if no corresponding pixel
///   format is found.
[[nodiscard]]
auto toDxgiFormat(const WICPixelFormatGUID &guid) noexcept -> DXGI_FORMAT {
    constexpr const struct {
        const WICPixelFormatGUID &wic;
        DXGI_FORMAT               dxgiFormat;
    } convertMap[]{
        {GUID_WICPixelFormat128bppRGBAFloat, DXGI_FORMAT_R32G32B32A32_FLOAT},
        {GUID_WICPixelFormat64bppRGBAHalf, DXGI_FORMAT_R16G16B16A16_FLOAT},
        {GUID_WICPixelFormat64bppRGBA, DXGI_FORMAT_R16G16B16A16_UNORM},
        {GUID_WICPixelFormat32bppRGBA, DXGI_FORMAT_R8G8B8A8_UNORM},
        {GUID_WICPixelFormat32bppBGRA, DXGI_FORMAT_B8G8R8A8_UNORM},
        {GUID_WICPixelFormat32bppBGR, DXGI_FORMAT_B8G8R8X8_UNORM},
        {GUID_WICPixelFormat32bppRGBA1010102XR, DXGI_FORMAT_R10G10B10_XR_BIAS_A2_UNORM},
        {GUID_WICPixelFormat32bppRGBA1010102, DXGI_FORMAT_R10G10B10A2_UNORM},
        {GUID_WICPixelFormat16bppBGRA5551, DXGI_FORMAT_B5G5R5A1_UNORM},
        {GUID_WICPixelFormat16bppBGR565, DXGI_FORMAT_B5G6R5_UNORM},
        {GUID_WICPixelFormat32bppGrayFloat, DXGI_FORMAT_R32_FLOAT},
        {GUID_WICPixelFormat16bppGrayHalf, DXGI_FORMAT_R16_FLOAT},
        {GUID_WICPixelFormat16bppGray, DXGI_FORMAT_R16_UNORM},
        {GUID_WICPixelFormat8bppGray, DXGI_FORMAT_R8_UNORM},
        {GUID_WICPixelFormat8bppAlpha, DXGI_FORMAT_A8_UNORM},
        {GUID_WICPixelFormatBlackWhite, DXGI_FORMAT_R1_UNORM},
    };

    for (const auto &c : convertMap) {
        if (c.wic == guid)
            return c.dxgiFormat;
    }

    return DXGI_FORMAT_UNKNOWN;
}

/// @brief
///   Get convert target WIC pixel format for the specified source pixel format.
///
/// @param src
///   The source WIC pixel format to be converted.
/// @param[out] dst
///   The destination WIC pixel format to be converted.
///
/// @return
///   A boolean value indicates whether the destination WIC pixel format is found.
/// @retval true
///   The destination WIC pixel format is found.
/// @retval false
///   The destination WIC pixel format is not found.
[[nodiscard]]
auto toWicConvertTargetFormat(const WICPixelFormatGUID &src, WICPixelFormatGUID &dst) noexcept
    -> bool {
    constexpr const struct {
        const WICPixelFormatGUID &source;
        const WICPixelFormatGUID &target;
    } convertMap[]{
        {GUID_WICPixelFormat1bppIndexed, GUID_WICPixelFormat32bppRGBA},
        {GUID_WICPixelFormat2bppIndexed, GUID_WICPixelFormat32bppRGBA},
        {GUID_WICPixelFormat4bppIndexed, GUID_WICPixelFormat32bppRGBA},
        {GUID_WICPixelFormat8bppIndexed, GUID_WICPixelFormat32bppRGBA},
        {GUID_WICPixelFormat2bppGray, GUID_WICPixelFormat8bppGray},
        {GUID_WICPixelFormat4bppGray, GUID_WICPixelFormat8bppGray},
        {GUID_WICPixelFormat16bppGrayFixedPoint, GUID_WICPixelFormat16bppGrayHalf},
        {GUID_WICPixelFormat32bppGrayFixedPoint, GUID_WICPixelFormat32bppGrayFloat},
        {GUID_WICPixelFormat16bppBGR555, GUID_WICPixelFormat16bppBGRA5551},
        {GUID_WICPixelFormat32bppBGR101010, GUID_WICPixelFormat32bppRGBA1010102},
        {GUID_WICPixelFormat24bppBGR, GUID_WICPixelFormat32bppRGBA},
        {GUID_WICPixelFormat24bppRGB, GUID_WICPixelFormat32bppRGBA},
        {GUID_WICPixelFormat32bppPBGRA, GUID_WICPixelFormat32bppRGBA},
        {GUID_WICPixelFormat32bppPRGBA, GUID_WICPixelFormat32bppRGBA},
        {GUID_WICPixelFormat48bppRGB, GUID_WICPixelFormat64bppRGBA},
        {GUID_WICPixelFormat48bppBGR, GUID_WICPixelFormat64bppRGBA},
        {GUID_WICPixelFormat64bppBGRA, GUID_WICPixelFormat64bppRGBA},
        {GUID_WICPixelFormat64bppPRGBA, GUID_WICPixelFormat64bppRGBA},
        {GUID_WICPixelFormat64bppPBGRA, GUID_WICPixelFormat64bppRGBA},
        {GUID_WICPixelFormat48bppRGBFixedPoint, GUID_WICPixelFormat64bppRGBAHalf},
        {GUID_WICPixelFormat48bppBGRFixedPoint, GUID_WICPixelFormat64bppRGBAHalf},
        {GUID_WICPixelFormat64bppRGBAFixedPoint, GUID_WICPixelFormat64bppRGBAHalf},
        {GUID_WICPixelFormat64bppBGRAFixedPoint, GUID_WICPixelFormat64bppRGBAHalf},
        {GUID_WICPixelFormat64bppRGBFixedPoint, GUID_WICPixelFormat64bppRGBAHalf},
        {GUID_WICPixelFormat64bppRGBHalf, GUID_WICPixelFormat64bppRGBAHalf},
        {GUID_WICPixelFormat48bppRGBHalf, GUID_WICPixelFormat64bppRGBAHalf},
        {GUID_WICPixelFormat128bppPRGBAFloat, GUID_WICPixelFormat128bppRGBAFloat},
        {GUID_WICPixelFormat128bppRGBFloat, GUID_WICPixelFormat128bppRGBAFloat},
        {GUID_WICPixelFormat128bppRGBAFixedPoint, GUID_WICPixelFormat128bppRGBAFloat},
        {GUID_WICPixelFormat128bppRGBFixedPoint, GUID_WICPixelFormat128bppRGBAFloat},
        {GUID_WICPixelFormat32bppRGBE, GUID_WICPixelFormat128bppRGBAFloat},
        {GUID_WICPixelFormat32bppCMYK, GUID_WICPixelFormat32bppRGBA},
        {GUID_WICPixelFormat64bppCMYK, GUID_WICPixelFormat64bppRGBA},
        {GUID_WICPixelFormat40bppCMYKAlpha, GUID_WICPixelFormat32bppRGBA},
        {GUID_WICPixelFormat80bppCMYKAlpha, GUID_WICPixelFormat64bppRGBA},
        {GUID_WICPixelFormat32bppRGB, GUID_WICPixelFormat32bppRGBA},
        {GUID_WICPixelFormat64bppRGB, GUID_WICPixelFormat64bppRGBA},
        {GUID_WICPixelFormat64bppPRGBAHalf, GUID_WICPixelFormat64bppRGBAHalf},
    };

    for (const auto &c : convertMap) {
        if (src == c.source) {
            dst = c.target;
            return true;
        }
    }

    return false;
}

/// @brief
///   Get pixel bit size for the specified pixel format.
///
/// @param factory
///   The WIC imaging factory to create WIC components.
/// @param format
///   The pixel format to get pixel size.
///
/// @return
///   Size in bit of the specified pixel format. Return -1 if failed to get pixel info.
[[nodiscard]]
auto wicPixelBitSize(IWICImagingFactory *factory, const WICPixelFormatGUID &format) noexcept
    -> std::uint32_t {
    ComPtr<IWICComponentInfo> compInfo;

    HRESULT hr = factory->CreateComponentInfo(format, compInfo.GetAddressOf());
    if (FAILED(hr))
        return std::uint32_t(-1);

    ComPtr<IWICPixelFormatInfo> pixelInfo;
    hr = compInfo.As(&pixelInfo);
    if (FAILED(hr))
        return std::uint32_t(-1);

    UINT bitSize;
    hr = pixelInfo->GetBitsPerPixel(&bitSize);
    if (FAILED(hr))
        return std::uint32_t(-1);

    return bitSize;
}

} // namespace

ink::Image::Image() noexcept
    : m_width(),
      m_height(),
      m_pixelFormat(DXGI_FORMAT_UNKNOWN),
      m_pixelBitSize(),
      m_rowPitch(),
      m_slicePitch(),
      m_data() {}

#if !defined(__clang__) && defined(_MSC_VER)
#    pragma warning(push)
#    pragma warning(disable : 6308 6387 28183)
#endif

ink::Image::Image(const Image &other) noexcept
    : m_width(other.m_width),
      m_height(other.m_height),
      m_pixelFormat(other.m_pixelFormat),
      m_pixelBitSize(other.m_pixelBitSize),
      m_rowPitch(other.m_rowPitch),
      m_slicePitch(other.m_slicePitch),
      m_data() {
    if (other.m_slicePitch == 0)
        return;

    m_data = std::malloc(m_slicePitch);
    std::memcpy(m_data, other.m_data, m_slicePitch);
}

auto ink::Image::operator=(const Image &other) noexcept -> Image & {
    if (this == &other)
        return *this;

    m_width        = other.m_width;
    m_height       = other.m_height;
    m_pixelFormat  = other.m_pixelFormat;
    m_pixelBitSize = other.m_pixelBitSize;
    m_rowPitch     = other.m_rowPitch;
    m_slicePitch   = other.m_slicePitch;

    if (m_slicePitch == 0) {
        if (m_data) {
            std::free(m_data);
            m_data = nullptr;
        }

        return *this;
    }

    if (m_data == nullptr)
        m_data = std::malloc(m_slicePitch);
    else
        m_data = std::realloc(m_data, m_slicePitch);

    std::memcpy(m_data, other.m_data, m_slicePitch);
    return *this;
}

ink::Image::Image(Image &&other) noexcept
    : m_width(other.m_width),
      m_height(other.m_height),
      m_pixelFormat(other.m_pixelFormat),
      m_pixelBitSize(other.m_pixelBitSize),
      m_rowPitch(other.m_rowPitch),
      m_slicePitch(other.m_slicePitch),
      m_data(other.m_data) {
    other.m_width        = 0;
    other.m_height       = 0;
    other.m_pixelFormat  = DXGI_FORMAT_UNKNOWN;
    other.m_pixelBitSize = 0;
    other.m_rowPitch     = 0;
    other.m_slicePitch   = 0;
    other.m_data         = nullptr;
}

auto ink::Image::operator=(Image &&other) noexcept -> Image & {
    if (m_data)
        std::free(m_data);

    m_width        = other.m_width;
    m_height       = other.m_height;
    m_pixelFormat  = other.m_pixelFormat;
    m_pixelBitSize = other.m_pixelBitSize;
    m_rowPitch     = other.m_rowPitch;
    m_slicePitch   = other.m_slicePitch;
    m_data         = other.m_data;

    other.m_width        = 0;
    other.m_height       = 0;
    other.m_pixelFormat  = DXGI_FORMAT_UNKNOWN;
    other.m_pixelBitSize = 0;
    other.m_rowPitch     = 0;
    other.m_slicePitch   = 0;
    other.m_data         = nullptr;

    return *this;
}

ink::Image::~Image() noexcept {
    if (m_data)
        std::free(m_data);
}

namespace {

struct ImageData {
    std::uint32_t width;
    std::uint32_t height;
    DXGI_FORMAT   pixelFormat;
    std::uint32_t pixelBitSize;
    std::size_t   rowPitch;
    std::size_t   slicePitch;
    void         *data;
};

[[nodiscard]]
auto loadImage(IWICImagingFactory *factory, IWICStream *stream, ImageData &data) noexcept -> bool {
    HRESULT hr;

    // Create image decoder.
    ComPtr<IWICBitmapDecoder> decoder;
    hr = factory->CreateDecoderFromStream(stream, nullptr, WICDecodeMetadataCacheOnDemand,
                                          decoder.GetAddressOf());
    if (FAILED(hr))
        return false;

    // Get image frame.
    ComPtr<IWICBitmapFrameDecode> frame;
    hr = decoder->GetFrame(0, frame.GetAddressOf());
    if (FAILED(hr))
        return false;

    WICPixelFormatGUID wicFormat;
    hr = frame->GetPixelFormat(&wicFormat);
    if (FAILED(hr))
        return false;

    // Get DXGI pixel format.
    ComPtr<IWICBitmapSource> bmp;
    data.pixelFormat = toDxgiFormat(wicFormat);

    // No corresponding DXGI format. Convert image pixel format.
    if (data.pixelFormat == DXGI_FORMAT_UNKNOWN) {
        // Get target image format.
        WICPixelFormatGUID dstFormat;
        if (!toWicConvertTargetFormat(wicFormat, dstFormat))
            return false;

        // Create converter.
        ComPtr<IWICFormatConverter> converter;
        hr = factory->CreateFormatConverter(converter.GetAddressOf());
        if (FAILED(hr))
            return false;

        hr = converter->Initialize(frame.Get(), dstFormat, WICBitmapDitherTypeNone, nullptr, 0.0f,
                                   WICBitmapPaletteTypeCustom);
        if (FAILED(hr))
            return false;

        // Get converted image.
        bmp              = converter;
        data.pixelFormat = toDxgiFormat(dstFormat);
        wicFormat        = dstFormat;
    } else {
        bmp = frame;
    }

    // Get metadata.
    hr = bmp->GetSize(&data.width, &data.height);
    if (FAILED(hr))
        return false;

    // Get pixel info.
    data.pixelBitSize = wicPixelBitSize(factory, wicFormat);
    if (data.pixelBitSize == std::uint32_t(-1))
        return false;

    // Row pitch size, rounded up to 1 byte.
    data.rowPitch   = (std::size_t(data.width) * data.pixelBitSize + 7) / 8;
    data.slicePitch = data.rowPitch * std::size_t(data.height);

    if (data.data != nullptr)
        data.data = std::realloc(data.data, data.slicePitch);
    else
        data.data = std::malloc(data.slicePitch);

    bmp->CopyPixels(nullptr, static_cast<UINT>(data.rowPitch), static_cast<UINT>(data.slicePitch),
                    static_cast<BYTE *>(data.data));

    return true;
}

} // namespace

#if !defined(__clang__) && defined(_MSC_VER)
#    pragma warning(pop)
#endif

auto ink::Image::load(StringView path) noexcept -> bool {
    HRESULT hr;

    // Create WIC data stream.
    IWICImagingFactory *const factory = ImageFactory::get();
    ComPtr<IWICStream>        stream;
    hr = factory->CreateStream(stream.GetAddressOf());
    if (FAILED(hr))
        return false;

    if (path.isNullTerminated()) {
        hr = stream->InitializeFromFilename(reinterpret_cast<LPCWSTR>(path.data()), GENERIC_READ);
    } else {
        String temp(path);
        hr = stream->InitializeFromFilename(reinterpret_cast<LPCWSTR>(temp.data()), GENERIC_READ);
    }

    if (FAILED(hr))
        return false;

    // Load image.
    ImageData imageData{};
    imageData.data = m_data;

    if (!loadImage(factory, stream.Get(), imageData))
        return false;

    m_width        = imageData.width;
    m_height       = imageData.height;
    m_pixelFormat  = imageData.pixelFormat;
    m_pixelBitSize = imageData.pixelBitSize;
    m_rowPitch     = imageData.rowPitch;
    m_slicePitch   = imageData.slicePitch;
    m_data         = imageData.data;

    return true;
}

auto ink::Image::load(const void *data, std::size_t size) noexcept -> bool {
    HRESULT hr;

    // Create WIC data stream.
    IWICImagingFactory *const factory = ImageFactory::get();
    ComPtr<IWICStream>        stream;
    hr = factory->CreateStream(stream.GetAddressOf());
    if (FAILED(hr))
        return false;

    hr = stream->InitializeFromMemory(const_cast<WICInProcPointer>(static_cast<const BYTE *>(data)),
                                      static_cast<DWORD>(size));
    if (FAILED(hr))
        return false;

    // Load image.
    ImageData imageData{};
    imageData.data = m_data;

    if (!loadImage(factory, stream.Get(), imageData))
        return false;

    m_width        = imageData.width;
    m_height       = imageData.height;
    m_pixelFormat  = imageData.pixelFormat;
    m_pixelBitSize = imageData.pixelBitSize;
    m_rowPitch     = imageData.rowPitch;
    m_slicePitch   = imageData.slicePitch;
    m_data         = imageData.data;

    return true;
}

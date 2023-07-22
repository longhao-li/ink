#pragma once

#include "ink/core/string.h"

#include <dxgiformat.h>

namespace ink {

class Image {
public:
    /// @brief
    ///   Create an empty image.
    InkApi Image() noexcept;

    /// @brief
    ///   Copy constructor of bitmap.
    ///
    /// @param other
    ///   The bitmap to be copied from.
    InkApi Image(const Image &other) noexcept;

    /// @brief
    ///   Copy assignment of bitmap.
    ///
    /// @param other
    ///   The bitmap to be copied from.
    ///
    /// @return
    ///   Reference to this bitmap.
    InkApi auto operator=(const Image &other) noexcept -> Image &;

    /// @brief
    ///   Move constructor of bitmap.
    ///
    /// @param other
    ///   The bitmap to be moved. The moved bitmap will be invalidated.
    InkApi Image(Image &&other) noexcept;

    /// @brief
    ///   Move assignment of bitmap.
    ///
    /// @param other
    ///   The bitmap to be moved. The moved bitmap will be invalidated.
    ///
    /// @return
    ///   Reference to this bitmap.
    InkApi auto operator=(Image &&other) noexcept -> Image &;

    /// @brief
    ///   Destroy this bitmap and release memory.
    InkApi ~Image() noexcept;

    /// @brief
    ///   Load an image from file.
    ///
    /// @param path
    ///   Path to the image file to be loaded.
    ///
    /// @return
    ///   A boolean value that indicates whether the load operation succeeded.
    /// @retval true
    ///   Succeeded to load the image file.
    /// @retval false
    ///   Failed to load the image file.
    InkApi auto load(StringView path) noexcept -> bool;

    /// @brief
    ///   Load a bitmap from memory.
    ///
    /// @param data
    ///   Pointer to start of the image data.
    /// @param size
    ///   Size in byte of the image.
    ///
    /// @return
    ///   A boolean value that indicates whether the load operation succeeded.
    /// @retval true
    ///   Succeeded to load the image.
    /// @retval false
    ///   Failed to load the image.
    InkApi auto load(const void *data, std::size_t size) noexcept -> bool;

    /// @brief
    ///   Get width in pixel of this image.
    ///
    /// @return
    ///   Width in pixel of this image.
    [[nodiscard]]
    auto width() const noexcept -> std::uint32_t {
        return m_width;
    }

    /// @brief
    ///   Get height in pixel of this image.
    ///
    /// @return
    ///   Height in pixel of this image.
    [[nodiscard]]
    auto height() const noexcept -> std::uint32_t {
        return m_height;
    }

    /// @brief
    ///   Get pixel format of this image.
    ///
    /// @return
    ///   Pixel format of this image.
    [[nodiscard]]
    auto pixelFormat() const noexcept -> DXGI_FORMAT {
        return m_pixelFormat;
    }

    /// @brief
    ///   Get number of bits per pixel. This value may not be a multiple of 8.
    ///
    /// @return
    ///   Number of bits per pixel.
    [[nodiscard]]
    auto pixelBitSize() const noexcept -> std::uint32_t {
        return m_pixelBitSize;
    }

    /// @brief
    ///   Get number of bytes per row of this image.
    /// @note
    ///   Since pixel bit size may not always be a multiple of 8, image row pitch may not be exactly
    ///   the same as image width x pixel size.
    ///
    /// @return
    ///   Number of bytes per row of this image.
    [[nodiscard]]
    auto rowPitch() const noexcept -> std::size_t {
        return m_rowPitch;
    }

    /// @brief
    ///   Get size in byte of this image. This value is exactly equal to rowPitch() x height().
    ///
    /// @return
    ///   Size in byte of this image.
    [[nodiscard]]
    auto size() const noexcept -> std::size_t {
        return m_slicePitch;
    }

    /// @brief
    ///   Get image data.
    ///
    /// @return
    ///   Pointer to start of the image data.
    [[nodiscard]]
    auto data() noexcept -> void * {
        return m_data;
    }

    /// @brief
    ///   Get image data.
    ///
    /// @return
    ///   Pointer to start of the image data.
    [[nodiscard]]
    auto data() const noexcept -> const void * {
        return m_data;
    }

private:
    /// @brief
    ///   Width in pixel of this bitmap.
    std::uint32_t m_width;

    /// @brief
    ///   Height in pixel of this bitmap.
    std::uint32_t m_height;

    /// @brief
    ///   Pixel format of this bitmap.
    DXGI_FORMAT m_pixelFormat;

    /// @brief
    ///   Size in bit of each pixel.
    std::uint32_t m_pixelBitSize;

    /// @brief
    ///   Size in byte of each row of image data.
    std::size_t m_rowPitch;

    /// @brief
    ///   Size in byte of this image.
    std::size_t m_slicePitch;

    /// @brief
    ///   Data of this bitmap.
    void *m_data;
};

} // namespace ink

#pragma once

#include "ink/core/error.h"

namespace ink {

class File {
public:
    /// @brief
    ///   File open mode flags.
    enum class Mode {
        None   = 0,
        Read   = 0x01,
        Write  = 0x02,
        Trunc  = 0x04,
        Append = 0x08,
    };

    /// @brief
    ///   Create a null file.
    InkApi File() noexcept;

    /// @brief
    ///   Open the specified file.
    /// @note
    ///   To open file without throwing exceptions, create a null file first and use @p File::open()
    ///   separately.
    ///
    /// @param path
    ///   Path to the file to be opened.
    /// @param mode
    ///   File open mode flags.
    ///
    /// @throw SystemErrorException
    ///   Thrown if failed to open the specified file.
    InkApi File(StringView path, Mode mode);

    /// @brief
    ///   Copy constructor of file is disabled.
    File(const File &) = delete;

    /// @brief
    ///   Copy assignment of file is disabled.
    auto operator=(const File &) = delete;

    /// @brief
    ///   Move constructor of file.
    ///
    /// @param other
    ///   The file to be moved. The moved file will be invalidated.
    InkApi File(File &&other) noexcept;

    /// @brief
    ///   Move assignment of file.
    ///
    /// @param other
    ///   The file to be moved. The moved file will be invalidated.
    ///
    /// @return
    ///   Reference to this file.
    InkApi auto operator=(File &&other) noexcept -> File &;

    /// @brief
    ///   Close file and destroy this object.
    InkApi ~File() noexcept;

    /// @brief
    ///   Try to open the specified file. This file object is not modified if failed to open the
    ///   specified file.
    ///
    /// @param path
    ///   Path to the file to be opened.
    /// @param mode
    ///   File access mode.
    ///
    /// @return
    ///   An error code that represents the open result.
    InkApi auto open(StringView path, Mode mode) noexcept -> ErrorCode;

    /// @brief
    ///   Close current file if opened.
    InkApi auto close() noexcept -> void;

    /// @brief
    ///   Checks if this file has been closed.
    ///
    /// @return
    ///   A boolean value that specifies whether this file is closed.
    /// @retval true
    ///   This file is closed.
    /// @retval false
    ///   This file is not closed.
    [[nodiscard]]
    auto isClosed() const noexcept -> bool {
        return m_fileHandle == reinterpret_cast<void *>(-1);
    }

    /// @brief
    ///   Get path of this file.
    ///
    /// @return
    ///   A string view that contains path of this file. It is guaranteed to be null-terminated.
    [[nodiscard]]
    auto path() const noexcept -> StringView {
        return m_path;
    }

    /// @brief
    ///   Seek the file pointer according to start of the file.
    ///
    /// @param offset
    ///   Expected offset to set the file pointer. Generally, this value should always be positive.
    ///
    /// @return
    ///   New position of the file pointer.
    InkApi auto seekBegin(std::int64_t offset) noexcept -> std::int64_t;

    /// @brief
    ///   Seek the file pointer according to current file pointer position.
    ///
    /// @param offset
    ///   Expected offset of the file pointer to be set.
    ///
    /// @return
    ///   New position of the file pointer.
    InkApi auto seekCurrent(std::int64_t offset) noexcept -> std::int64_t;

    /// @brief
    ///   Seek the file pointer according to end of current file.
    ///
    /// @param offset
    ///   Expected offset of the file pointer to be set.
    ///
    /// @return
    ///   New position of the file pointer.
    InkApi auto seekEnd(std::int64_t offset) noexcept -> std::int64_t;

    /// @brief
    ///   Get size in byte of this file.
    ///
    /// @return
    ///   Size in byte of this file.
    [[nodiscard]]
    InkApi auto size() const noexcept -> std::size_t;

    /// @brief
    ///   Read data to the specified buffer.
    ///
    /// @param[out] buffer
    ///   Pointer to start of the buffer to store data.
    /// @param size
    ///   Expected size in byte of data to read.
    ///
    /// @return
    ///   An error code that represents result of the read operation.
    InkApi auto read(void *buffer, std::uint32_t size) noexcept -> ErrorCode;

    /// @brief
    ///   Read data to the specified buffer.
    ///
    /// @param[out] buffer
    ///   Pointer to start of the buffer to store data.
    /// @param size
    ///   Expected size in byte of data to read.
    /// @param[out] bytesRead
    ///   Bytes read from file.
    ///
    /// @return
    ///   An error code that represents result of the read operation.
    InkApi auto read(void *buffer, std::uint32_t size, std::uint32_t &bytesRead) noexcept
        -> ErrorCode;

    /// @brief
    ///   Write the specified data to this file.
    ///
    /// @param buffer
    ///   Pointer to start of data to be written.
    /// @param size
    ///   Size in byte of data to be written.
    ///
    /// @return
    ///   An error code that represents result of the write operation.
    InkApi auto write(const void *buffer, std::uint32_t size) noexcept -> ErrorCode;

    /// @brief
    ///   Write the specified data to this file.
    ///
    /// @param buffer
    ///   Pointer to start of data to be written.
    /// @param size
    ///   Size in byte of data to be written.
    /// @param[out] bytesWritten
    ///   Bytes written to file.
    ///
    /// @return
    ///   An error code that represents result of the write operation.
    InkApi auto write(const void *buffer, std::uint32_t size, std::uint32_t &bytesWritten) noexcept
        -> ErrorCode;

    /// @brief
    ///   Flush this file.
    InkApi auto flush() noexcept -> void;

    /// @brief
    ///   Map this file to memory. The file mapping will be recreated if @p size is greater than
    ///   current file mapping size.
    ///
    /// @param size
    ///   Expected size in byte to be mapped. For writable files, this value could be larger than
    ///   file size so that the file mapping could append data to the end of the file.
    ///
    /// @return
    ///   Pointer to start of the mapped memory. @p nullptr will be returned if the file mapping
    ///   failed.
    [[nodiscard]]
    InkApi auto map(std::size_t size) noexcept -> void *;

    /// @brief
    ///   Get size in byte of the file mapping.
    ///
    /// @return
    ///   Size in byte of the file mapping.
    [[nodiscard]]
    auto mappingSize() const noexcept -> std::size_t {
        return m_mappedSize;
    }

    /// @brief
    ///   Checks if this file is readable.
    ///
    /// @return
    ///   A boolean value that indicates whether this file is readable.
    /// @retval true
    ///   This file is readable.
    /// @retval false
    ///   This file is not readable.
    [[nodiscard]]
    auto isReadable() const noexcept -> bool {
        return (static_cast<int>(m_mode) & static_cast<int>(Mode::Read)) != 0;
    }

    /// @brief
    ///   Checks if this file is writable.
    ///
    /// @return
    ///   A boolean value that indicates whether this file is writable.
    /// @retval true
    ///   This file is writable.
    /// @retval false
    ///   This file is not writable.
    [[nodiscard]]
    auto isWritable() const noexcept -> bool {
        return (static_cast<int>(m_mode) & static_cast<int>(Mode::Write)) != 0;
    }

private:
    /// @brief
    ///   Path of this file.
    String m_path;

    /// @brief
    ///   Open mode of this file.
    Mode m_mode;

    /// @brief
    ///   Win32 file handle.
    void *m_fileHandle;

    /// @brief
    ///   File mapping object of this file. This is used only if file mapping is required.
    void *m_fileMapping;

    /// @brief
    ///   Pointer to start of the mapped memory.
    void *m_mappingData;

    /// @brief
    ///   Size in byte of the mapped memory.
    std::size_t m_mappedSize;
};

constexpr auto operator~(File::Mode a) noexcept -> File::Mode {
    return static_cast<File::Mode>(~static_cast<int>(a));
}

constexpr auto operator|(File::Mode a, File::Mode b) noexcept -> File::Mode {
    return static_cast<File::Mode>(static_cast<int>(a) | static_cast<int>(b));
}

constexpr auto operator&(File::Mode a, File::Mode b) noexcept -> File::Mode {
    return static_cast<File::Mode>(static_cast<int>(a) & static_cast<int>(b));
}

constexpr auto operator^(File::Mode a, File::Mode b) noexcept -> File::Mode {
    return static_cast<File::Mode>(static_cast<int>(a) ^ static_cast<int>(b));
}

constexpr auto operator|=(File::Mode &a, File::Mode b) noexcept -> File::Mode & {
    a = (a | b);
    return a;
}

constexpr auto operator&=(File::Mode &a, File::Mode b) noexcept -> File::Mode & {
    a = (a & b);
    return a;
}

constexpr auto operator^=(File::Mode &a, File::Mode b) noexcept -> File::Mode & {
    a = (a ^ b);
    return a;
}

} // namespace ink

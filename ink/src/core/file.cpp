#include "ink/core/file.h"

#include <Windows.h>

using namespace ink;

ink::File::File() noexcept
    : m_path(),
      m_mode(),
      m_fileHandle(INVALID_HANDLE_VALUE),
      m_fileMapping(nullptr),
      m_mappingData(nullptr),
      m_mappedSize() {}

ink::File::File(StringView path, Mode mode)
    : m_path(path),
      m_mode(mode),
      m_fileHandle(INVALID_HANDLE_VALUE),
      m_fileMapping(nullptr),
      m_mappingData(nullptr),
      m_mappedSize() {
    DWORD accessFlags   = 0;
    DWORD creationFlags = 0;

    if ((mode & Mode::Read) != Mode::None) {
        accessFlags |= GENERIC_READ;
        creationFlags = CREATE_ALWAYS;
    }

    if ((mode & Mode::Write) != Mode::None) {
        accessFlags |= GENERIC_WRITE;
        creationFlags = OPEN_ALWAYS;
        if ((mode & Mode::Trunc) != Mode::None)
            creationFlags = CREATE_ALWAYS;
    }

    DWORD shareMode = 0;
    if ((mode & (Mode::Read | Mode::Write)) == Mode::Read)
        shareMode = FILE_SHARE_READ;

    m_fileHandle = CreateFileW(reinterpret_cast<LPCWSTR>(m_path.data()), accessFlags, shareMode,
                               nullptr, creationFlags, FILE_ATTRIBUTE_NORMAL, nullptr);

    if (m_fileHandle == INVALID_HANDLE_VALUE)
        throw SystemErrorException(GetLastError(), format(u"Failed to open file {}", path));
}

ink::File::File(File &&other) noexcept
    : m_path(std::move(other.m_path)),
      m_mode(other.m_mode),
      m_fileHandle(other.m_fileHandle),
      m_fileMapping(other.m_fileMapping),
      m_mappingData(other.m_mappingData),
      m_mappedSize(other.m_mappedSize) {
    other.m_mode        = Mode::None;
    other.m_fileHandle  = INVALID_HANDLE_VALUE;
    other.m_fileMapping = nullptr;
    other.m_mappingData = nullptr;
    other.m_mappedSize  = 0;
}

auto ink::File::operator=(File &&other) noexcept -> File & {
    this->close();

    m_path        = std::move(other.m_path);
    m_mode        = other.m_mode;
    m_fileHandle  = other.m_fileHandle;
    m_fileMapping = other.m_fileMapping;
    m_mappingData = other.m_mappingData;
    m_mappedSize  = other.m_mappedSize;

    other.m_mode        = Mode::None;
    other.m_fileHandle  = INVALID_HANDLE_VALUE;
    other.m_fileMapping = nullptr;
    other.m_mappingData = nullptr;
    other.m_mappedSize  = 0;

    return *this;
}

ink::File::~File() noexcept {
    this->close();
}

auto ink::File::open(StringView path, Mode mode) noexcept -> ErrorCode {
    String tempPath(path);
    DWORD  accessFlags   = 0;
    DWORD  creationFlags = 0;

    if ((mode & Mode::Read) != Mode::None) {
        accessFlags |= GENERIC_READ;
        creationFlags = CREATE_ALWAYS;
    }

    if ((mode & Mode::Write) != Mode::None) {
        accessFlags |= GENERIC_WRITE;
        creationFlags = OPEN_ALWAYS;
        if ((mode & Mode::Trunc) != Mode::None)
            creationFlags = CREATE_ALWAYS;
    }

    DWORD shareMode = 0;
    if ((mode & (Mode::Read | Mode::Write)) == Mode::Read)
        shareMode = FILE_SHARE_READ;

    HANDLE newHandle =
        CreateFileW(reinterpret_cast<LPCWSTR>(tempPath.data()), accessFlags, shareMode, nullptr,
                    creationFlags, FILE_ATTRIBUTE_NORMAL, nullptr);
    if (newHandle == INVALID_HANDLE_VALUE)
        return ErrorCode(static_cast<std::int32_t>(GetLastError()),
                         SystemErrorCategory::singleton());

    this->close();

    this->m_path       = std::move(tempPath);
    this->m_mode       = mode;
    this->m_fileHandle = newHandle;

    return ErrorCode();
}

auto ink::File::close() noexcept -> void {
    if (m_fileMapping != nullptr) {
        CloseHandle(m_fileMapping);

        m_mappedSize  = 0;
        m_mappingData = nullptr;
        m_fileMapping = nullptr;
    }

    if (m_fileHandle != INVALID_HANDLE_VALUE) {
        CloseHandle(m_fileHandle);

        m_fileHandle = INVALID_HANDLE_VALUE;
        m_mode       = Mode::None;
        m_path.clear();
    }
}

auto ink::File::seekBegin(std::int64_t offset) noexcept -> std::int64_t {
    if (isClosed())
        return 0;

    LARGE_INTEGER off;
    off.QuadPart = offset;

    LARGE_INTEGER newPos;
    newPos.QuadPart = 0;

    if (SetFilePointerEx(m_fileHandle, off, &newPos, FILE_BEGIN))
        return newPos.QuadPart;

    return 0;
}

auto ink::File::seekCurrent(std::int64_t offset) noexcept -> std::int64_t {
    if (isClosed())
        return 0;

    LARGE_INTEGER off;
    off.QuadPart = offset;

    LARGE_INTEGER newPos;
    newPos.QuadPart = 0;

    if (SetFilePointerEx(m_fileHandle, off, &newPos, FILE_CURRENT))
        return newPos.QuadPart;

    return 0;
}

auto ink::File::seekEnd(std::int64_t offset) noexcept -> std::int64_t {
    if (isClosed())
        return 0;

    LARGE_INTEGER off;
    off.QuadPart = offset;

    LARGE_INTEGER newPos;
    newPos.QuadPart = 0;

    if (SetFilePointerEx(m_fileHandle, off, &newPos, FILE_END))
        return newPos.QuadPart;

    return 0;
}

auto ink::File::size() const noexcept -> std::size_t {
    if (isClosed())
        return 0;

    LARGE_INTEGER sz;
    if (GetFileSizeEx(m_fileHandle, &sz))
        return static_cast<std::size_t>(sz.QuadPart);

    return 0;
}

auto ink::File::read(void *buffer, std::uint32_t size) noexcept -> ErrorCode {
    if (ReadFile(m_fileHandle, buffer, size, nullptr, nullptr))
        return ErrorCode();

    return ErrorCode(static_cast<std::int32_t>(GetLastError()), SystemErrorCategory::singleton());
}

auto ink::File::read(void *buffer, std::uint32_t size, std::uint32_t &bytesRead) noexcept
    -> ErrorCode {
    DWORD readSize;
    if (ReadFile(m_fileHandle, buffer, size, &readSize, nullptr)) {
        bytesRead = readSize;
        return ErrorCode();
    }

    return ErrorCode(static_cast<std::int32_t>(GetLastError()), SystemErrorCategory::singleton());
}

auto ink::File::write(const void *buffer, std::uint32_t size) noexcept -> ErrorCode {
    if (WriteFile(m_fileHandle, buffer, size, nullptr, nullptr))
        return ErrorCode();

    return ErrorCode(static_cast<std::int32_t>(GetLastError()), SystemErrorCategory::singleton());
}

InkApi auto ink::File::write(const void    *buffer,
                             std::uint32_t  size,
                             std::uint32_t &bytesWritten) noexcept -> ErrorCode {
    DWORD writeSize;
    if (WriteFile(m_fileHandle, buffer, size, &writeSize, nullptr)) {
        bytesWritten = writeSize;
        return ErrorCode();
    }

    return ErrorCode(static_cast<std::int32_t>(GetLastError()), SystemErrorCategory::singleton());
}

auto ink::File::flush() noexcept -> void {
    if (isClosed())
        return;

    FlushFileBuffers(m_fileHandle);
}

auto ink::File::map(std::size_t size) noexcept -> void * {
    if (isClosed())
        return nullptr;

    if (size <= m_mappedSize)
        return m_mappingData;

    if (m_fileMapping != nullptr) {
        CloseHandle(m_fileMapping);

        m_mappedSize  = 0;
        m_mappingData = nullptr;
        m_fileMapping = nullptr;
    }

    const bool writable     = ((m_mode & Mode::Write) == Mode::Write);
    bool       useLargePage = false;

    { // Create file mapping.
        DWORD pageAttr = SEC_COMMIT;
        pageAttr |= (writable ? PAGE_READWRITE : PAGE_READONLY);

        const std::size_t largePageSize = GetLargePageMinimum();
        if (writable && size >= largePageSize * 8) {
            useLargePage = true;
            // Align up map size
            size = ((size + largePageSize - 1) / largePageSize * largePageSize);
            pageAttr |= SEC_LARGE_PAGES;
        }

        LARGE_INTEGER mapSize;
        mapSize.QuadPart = size;
        m_fileMapping    = CreateFileMappingW(m_fileHandle, nullptr, pageAttr, mapSize.HighPart,
                                              mapSize.LowPart, nullptr);
        if (m_fileMapping == nullptr)
            return nullptr;
    }

    { // Map view of file.
        DWORD access = writable ? FILE_MAP_ALL_ACCESS : FILE_MAP_READ;
        if (useLargePage)
            access |= FILE_MAP_LARGE_PAGES;

        m_mappingData = MapViewOfFileEx(m_fileMapping, access, 0, 0, size, nullptr);

        // Failed to map data, revert.
        if (m_mappingData == nullptr) {
            CloseHandle(m_fileMapping);
            m_fileMapping = nullptr;
            return nullptr;
        }
    }

    m_mappedSize = size;
    return m_mappingData;
}

#include "ink/core/exception.h"

#include <Windows.h>

using namespace ink;

ink::Exception::Exception() noexcept : m_sourceLocation(), m_errorMessage() {}

ink::Exception::Exception(SourceLocation loc) noexcept : m_sourceLocation(loc), m_errorMessage() {}

ink::Exception::Exception(String message) noexcept
    : m_sourceLocation(), m_errorMessage(std::move(message)) {}

ink::Exception::Exception(SourceLocation loc, String message) noexcept
    : m_sourceLocation(loc), m_errorMessage(std::move(message)) {}

ink::Exception::~Exception() noexcept {}

auto ink::Exception::what() const noexcept -> const char * {
    return "Deprecated. Use errorMessage() instead.";
}

auto ink::Exception::errorMessage() const noexcept -> StringView {
    return m_errorMessage;
}

namespace {

auto errorToString(std::int32_t errc) noexcept -> String {
    char16_t buffer[4096];
    DWORD    charCount =
        FormatMessageW(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, nullptr, errc,
                       MAKELANGID(LANG_NEUTRAL, SUBLANG_NEUTRAL), reinterpret_cast<LPWSTR>(buffer),
                       static_cast<DWORD>(_countof(buffer)), nullptr);
    if (charCount <= 0)
        return String(u"Unknown system error.");

    StringView msg(buffer, static_cast<std::size_t>(charCount));
    msg.trim();

    return String(msg);
}

} // namespace

ink::SystemErrorException::SystemErrorException() noexcept : Exception(), m_errorCode() {}

ink::SystemErrorException::SystemErrorException(std::int32_t errc) noexcept
    : Exception(errorToString(errc)), m_errorCode(errc) {}

ink::SystemErrorException::SystemErrorException(std::int32_t errc, String message) noexcept
    : Exception(std::move(message)), m_errorCode(errc) {}

ink::SystemErrorException::SystemErrorException(SourceLocation loc, std::int32_t errc) noexcept
    : Exception(loc, errorToString(errc)), m_errorCode(errc) {}

ink::SystemErrorException::SystemErrorException(SourceLocation loc,
                                                std::int32_t   errc,
                                                String         message) noexcept
    : Exception(loc, std::move(message)), m_errorCode(errc) {}

ink::SystemErrorException::~SystemErrorException() noexcept {}

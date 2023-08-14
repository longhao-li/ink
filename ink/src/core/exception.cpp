#include "ink/core/exception.hpp"

using namespace ink;

ink::Exception::Exception() noexcept = default;

ink::Exception::Exception(std::string msg) noexcept : std::exception(), m_message(std::move(msg)) {}

ink::Exception::~Exception() noexcept = default;

auto ink::Exception::what() const noexcept -> const char * { return m_message.c_str(); }

namespace {

/// @brief
///   Convert error code to system error message.
///
/// @param errc
///   System error code. Could either be a Windows error code or HRESULT.
[[nodiscard]] auto errcToString(std::int32_t errc) noexcept -> std::string {
    std::array<char, 4096> buffer;

    DWORD charCount =
        FormatMessageA(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, nullptr,
                       static_cast<DWORD>(errc), MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US),
                       buffer.data(), static_cast<DWORD>(buffer.size()), nullptr);
    if (charCount <= 0)
        return {"Unknown system error."};

    std::string result(buffer.data(), static_cast<std::size_t>(charCount));
    while (!result.empty() && std::isspace(result.back()))
        result.pop_back();

    return result;
}

} // namespace

ink::SystemErrorException::SystemErrorException() noexcept
    : Exception(errcToString(S_OK)), m_errorCode(0) {}

ink::SystemErrorException::SystemErrorException(std::int32_t errc) noexcept
    : Exception(errcToString(errc)), m_errorCode(errc) {}

ink::SystemErrorException::SystemErrorException(std::int32_t errc, std::string msg) noexcept
    : Exception(std::move(msg)), m_errorCode(errc) {
    std::string sysErrMsg = errcToString(errc);
    m_message.reserve(m_message.size() + sysErrMsg.size() + 2);
    m_message.push_back(' ');
    m_message.append(sysErrMsg);
}

ink::SystemErrorException::~SystemErrorException() noexcept = default;

ink::RenderAPIException::~RenderAPIException() noexcept = default;

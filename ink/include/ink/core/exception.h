#pragma once

#include "ink/core/string.h"

namespace ink {

struct SourceLocation {
    const char16_t *file;
    int             line;
    const char16_t *func;
};

/// @brief
///   Base class for all ink exceptions.
/// @remark
///   This class inherits from @p std::exception so that you could catch all exceptions by simply
///   catching @p std::exception.
class Exception : public std::exception {
public:
    /// @brief
    ///   Create an empty exception object.
    Exception() noexcept;

    /// @brief
    ///   Create an exception object with source location.
    /// @note
    ///   The error message will be leave empty.
    ///
    /// @param loc
    ///   Source location of the exception.
    Exception(SourceLocation loc) noexcept;

    /// @brief
    ///   Create an exception object with error message.
    /// @note
    ///   The source location will be leave empty.
    ///
    /// @param message
    ///   Error message of this exception.
    Exception(String message) noexcept;

    /// @brief
    ///   Create an exception with source location and error message.
    ///
    /// @param loc
    ///   Source location of this exception.
    /// @param message
    ///   Error message of this exception.
    Exception(SourceLocation loc, String message) noexcept;

    /// @brief
    ///   Destroy this exception object.
    ~Exception() noexcept override;

    [[deprecated("This method doesn't return any useful message. Use errorMessage() instead.")]]
    auto what() const noexcept -> const char * final;

    /// @brief
    ///   Get source location of this exception.
    ///
    /// @return
    ///   Source location of this exception.
    [[nodiscard]]
    auto sourceLocation() const noexcept -> SourceLocation {
        return m_sourceLocation;
    }

    /// @brief
    ///   Get error message of this exception.
    ///
    /// @return
    ///   Error message of this exception.
    [[nodiscard]]
    auto errorMessage() const noexcept -> StringView;

protected:
    /// @brief
    ///   Error source location information.
    SourceLocation m_sourceLocation;

    /// @brief
    ///   Error message of this exception.
    String m_errorMessage;
};

class SystemErrorException : public Exception {
public:
    /// @brief
    ///   Create an empty system error exception. Error code is set to 0.
    SystemErrorException() noexcept;

    /// @brief
    ///   Create a system error exception with system error code.
    /// @note
    ///   The error message will be set according to the error code.
    ///
    /// @param errc
    ///   System error code of this exception.
    SystemErrorException(std::int32_t errc) noexcept;

    /// @brief
    ///   Create a system error exception with system error code and custom error message.
    ///
    /// @param errc
    ///   System error code of this exception.
    /// @param message
    ///   Custom error message.
    SystemErrorException(std::int32_t errc, String message) noexcept;

    /// @brief
    ///   Create a system error exception with source location and error code.
    /// @note
    ///   The error message will be set according to the error code.
    ///
    /// @param loc
    ///   Source location of this system error exception.
    /// @param errc
    ///   System error code of this exception.
    SystemErrorException(SourceLocation loc, std::int32_t errc) noexcept;

    /// @brief
    ///   Create a system error exception with source location, system error code and custom error
    ///   message.
    ///
    /// @param loc
    ///   Source location of this system error exception.
    /// @param errc
    ///   System error code of this exception.
    /// @param message
    ///   Custom error message.
    SystemErrorException(SourceLocation loc, std::int32_t errc, String message) noexcept;

    /// @brief
    ///   Destroy this system error exception.
    ~SystemErrorException() noexcept override;

    /// @brief
    ///   Get system error code from this exception.
    /// @remark
    ///   Using win32 API @p FormatMessage could convert the error code to string. Do not use @p
    ///   strerror for win32 error code.
    ///
    /// @return
    ///   System error code of this exception.
    [[nodiscard]]
    auto errorCode() const noexcept -> std::int32_t {
        return m_errorCode;
    }

protected:
    /// @brief
    ///   System error code of this exception.
    std::int32_t m_errorCode;
};

} // namespace ink

template <>
struct fmt::formatter<ink::SourceLocation, char16_t> : fmt::formatter<ink::StringView, char16_t> {
    template <typename FormatContext>
    auto format(ink::SourceLocation loc, FormatContext &ctx) const -> decltype(ctx.out()) {
        if (loc.file && loc.func)
            return ink::formatTo(ctx.out(), u"{}:{} {}", loc.file, loc.line, loc.func);
        else if (loc.file)
            return ink::formatTo(ctx.out(), u"{}:{}", loc.file, loc.line);
        else if (loc.func)
            return ink::formatTo(ctx.out(), u"{}", loc.func);
        else
            return ctx.out();
    }
};

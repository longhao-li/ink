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
    InkApi Exception() noexcept;

    /// @brief
    ///   Create an exception object with source location.
    /// @note
    ///   The error message will be leave empty.
    ///
    /// @param loc
    ///   Source location of the exception.
    InkApi Exception(SourceLocation loc) noexcept;

    /// @brief
    ///   Create an exception object with error message.
    /// @note
    ///   The source location will be leave empty.
    ///
    /// @param message
    ///   Error message of this exception.
    InkApi Exception(String message) noexcept;

    /// @brief
    ///   Create an exception with source location and error message.
    ///
    /// @param loc
    ///   Source location of this exception.
    /// @param message
    ///   Error message of this exception.
    InkApi Exception(SourceLocation loc, String message) noexcept;

    /// @brief
    ///   Destroy this exception object.
    InkApi ~Exception() noexcept override;

    [[deprecated("This method doesn't return any useful message. Use errorMessage() instead.")]]
    InkApi auto what() const noexcept -> const char * final;

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
    InkApi auto errorMessage() const noexcept -> StringView;

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
    InkApi SystemErrorException() noexcept;

    /// @brief
    ///   Create a system error exception with system error code.
    /// @note
    ///   The error message will be set according to the error code.
    ///
    /// @param errc
    ///   System error code of this exception.
    InkApi SystemErrorException(std::int32_t errc) noexcept;

    /// @brief
    ///   Create a system error exception with system error code and custom error message.
    ///
    /// @param errc
    ///   System error code of this exception.
    /// @param message
    ///   Custom error message.
    InkApi SystemErrorException(std::int32_t errc, String message) noexcept;

    /// @brief
    ///   Create a system error exception with source location and error code.
    /// @note
    ///   The error message will be set according to the error code.
    ///
    /// @param loc
    ///   Source location of this system error exception.
    /// @param errc
    ///   System error code of this exception.
    InkApi SystemErrorException(SourceLocation loc, std::int32_t errc) noexcept;

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
    InkApi SystemErrorException(SourceLocation loc, std::int32_t errc, String message) noexcept;

    /// @brief
    ///   Destroy this system error exception.
    InkApi ~SystemErrorException() noexcept override;

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

namespace ink {

class ErrorCategory {
public:
    /// @brief
    ///   Create an error category.
    ErrorCategory() noexcept = default;

    /// @brief
    ///   Copy constructor of error category is disabled.
    ErrorCategory(const ErrorCategory &) = delete;

    /// @brief
    ///   Copy assignment of error category is disabled.
    auto operator=(const ErrorCategory &) = delete;

    /// @brief
    ///   Destroy this error category.
    virtual ~ErrorCategory() noexcept = default;

    /// @brief
    ///   Get name of this error category.
    ///
    /// @return
    ///   Name of this error category.
    [[nodiscard]]
    virtual auto name() const noexcept -> StringView = 0;

    /// @brief
    ///  Convert the specified error code to error message according to this error category.
    ///
    /// @param errorCode
    ///   The error code to get the error message.
    ///
    /// @return
    ///   A string that contains error message of the error code.
    [[nodiscard]]
    virtual auto toMessage(std::int32_t errorCode) const noexcept -> String = 0;
};

class SystemErrorCategory : public ErrorCategory {
public:
    /// @brief
    ///   Create a system error category.
    SystemErrorCategory() noexcept = default;

    /// @brief
    ///   Destroy this system error category.
    InkApi ~SystemErrorCategory() noexcept override;

    /// @brief
    ///   Get name of system error category.
    ///
    /// @return
    ///   Name of system error category.
    [[nodiscard]]
    InkApi auto name() const noexcept -> StringView override;

    /// @brief
    ///  Convert the specified error code to system error message.
    ///
    /// @param errorCode
    ///   The error code to get the error message.
    ///
    /// @return
    ///   A string that contains error message of the error code.
    [[nodiscard]]
    InkApi auto toMessage(std::int32_t errorCode) const noexcept -> String override;

    /// @brief
    ///   Get system error category singleton.
    ///
    /// @return
    ///   System error category singleton instance.
    InkApi static auto singleton() noexcept -> const SystemErrorCategory &;
};

class ErrorCode {
public:
    /// @brief
    ///   Create an empty error code. This is equivalent to empty system error.
    ErrorCode() noexcept : m_errorCode(), m_category(&SystemErrorCategory::singleton()) {}

    /// @brief
    ///   Create a new error code for the specified error category.
    ///
    /// @param code
    ///   Error code value.
    /// @param category
    ///   Error category of this error code.
    ErrorCode(std::int32_t code, const ErrorCategory &category) noexcept
        : m_errorCode(code), m_category(&category) {}

    /// @brief
    ///   Checks if this error code represents no error.
    ///
    /// @return
    ///   A boolean value that indicates whether this error code represents no error.
    /// @retval true
    ///   This error code represents no error.
    /// @retval false
    ///   This error code represents a certain type of error.
    [[nodiscard]]
    auto isOk() const noexcept -> bool {
        return m_errorCode == 0;
    }

    /// @brief
    ///   Get value of this error code.
    ///
    /// @return
    ///   Value of this error code.
    [[nodiscard]]
    auto value() const noexcept -> std::int32_t {
        return m_errorCode;
    }

    /// @brief
    ///   Get error message of this error code.
    ///
    /// @return
    ///   Error message that this error code represents.
    [[nodiscard]]
    auto toMessage() const noexcept -> String {
        return m_category->toMessage(m_errorCode);
    }

    /// @brief
    ///   Set this error code to 0 in system error category.
    auto clear() noexcept -> void {
        m_errorCode = 0;
        m_category  = &SystemErrorCategory::singleton();
    }

private:
    /// @brief
    ///   Error code value.
    std::int32_t m_errorCode;

    /// @brief
    ///   Error category of this error code.
    const ErrorCategory *m_category;
};

} // namespace ink

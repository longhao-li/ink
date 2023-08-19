#pragma once

#include "export.hpp"

#include <string>

namespace ink {

class Exception : public std::exception {
public:
    /// @brief
    ///   Create an exception with no error message.
    InkExport Exception() noexcept;

    /// @brief
    ///   Create an exception object with the specified error message.
    ///
    /// @param msg
    ///   Error message of this exception.
    InkExport Exception(std::string msg) noexcept;

    /// @brief
    ///   Destroy this exception object.
    InkExport ~Exception() noexcept override;

    /// @brief
    ///   Get error message of this exception.
    ///
    /// @return
    ///   Return a C style string that represents the error message of this exception.
    [[nodiscard]] InkExport auto what() const noexcept -> const char * override;

protected:
    /// @brief
    ///   Error message of this exception object.
    std::string m_message;
};

class SystemErrorException : public Exception {
public:
    /// @brief
    ///   Create an exception with no error message.
    InkExport SystemErrorException() noexcept;

    /// @brief
    ///   Create a system error exception with system error code. The error message will be
    ///   determined by the system error code.
    ///
    /// @param errc
    ///   System error code of this exception.
    InkExport SystemErrorException(std::int32_t errc) noexcept;

    /// @brief
    ///   Create a system error exception with system error code and error message.
    ///
    /// @param errc
    ///   System error code of this exception.
    /// @param msg
    ///   Error message of this exception.
    InkExport SystemErrorException(std::int32_t errc, std::string msg) noexcept;

    /// @brief
    ///   Destroy this exception object.
    InkExport ~SystemErrorException() noexcept override;

    /// @brief
    ///   Get system error code of this exception.
    ///
    /// @return
    ///   System error code of this exception.
    [[nodiscard]] auto errorCode() const noexcept -> std::int32_t { return m_errorCode; }

protected:
    /// @brief
    ///   System error code of this exception.
    std::int32_t m_errorCode;
};

class RenderAPIException : public SystemErrorException {
public:
    using SystemErrorException::SystemErrorException;

    /// @brief
    ///   Destroy this exception object.
    InkExport ~RenderAPIException() noexcept override;
};

} // namespace ink

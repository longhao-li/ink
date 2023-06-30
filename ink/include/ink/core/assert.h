#pragma once

#include "ink/core/log.h"

#include <cassert>

namespace ink::detail {

#ifndef NDEBUG
/// @brief
///   Trigger a debug break for MSVC debugger.
///
/// @param file
///   Source file at which the debug break is triggered.
/// @param line
///   Source line in the source file.
inline auto assertFailed(const char16_t *file, std::uint32_t line) noexcept -> void {
    logFatal(u"Assertion failed at {}:{}.", file, line);
    _wassert(L"Fatal Error", reinterpret_cast<const wchar_t *>(file), line);
}

/// @brief
///   Format the error message and trigger a debug break for MSVC debugger.
/// @note
///   This function should never throw any exceptions and marked as noexcept. Therefore, formatters
///   should never throw exceptions or program will be terminated immediately.
///
/// @tparam ...Args
///   Types of arguments to be formatted.
///
/// @param file
///   Source file at which the debug break is triggered.
/// @param line
///   Source line in the source file.
/// @param fmt
///   Error message format pattern string.
/// @param ...args
///   Arguments to be formatted.
template <typename... Args>
auto assertFailed(const char16_t       *file,
                  std::uint32_t         line,
                  FormatString<Args...> fmt,
                  Args &&...args) noexcept -> void {
    FormatMemoryBuffer buffer;
    formatTo(std::back_inserter(buffer), fmt, std::forward<Args>(args)...);
    buffer.push_back(char16_t());
    logFatal(u"Assertion failed at {}:{} -- {}", file, line,
             StringView(buffer.data(), buffer.size()));
    _wassert(reinterpret_cast<const wchar_t *>(buffer.data()),
             reinterpret_cast<const wchar_t *>(file), line);
}
#endif

} // namespace ink::detail

#ifdef NDEBUG
#    define inkAssert(exp, ...) ((void)0)
#else
#    define inkAssert(exp, ...)                                                                    \
        (void)((!!(exp)) ||                                                                        \
               (::ink::detail::assertFailed(INK_UTF16(__FILE__),                                   \
                                            static_cast<::std::uint32_t>(__LINE__), __VA_ARGS__),  \
                0))
#endif

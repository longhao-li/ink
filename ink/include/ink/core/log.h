#pragma once

#include "ink/core/string.h"

#include <fmt/chrono.h>

namespace ink {

enum class LogLevel {
    Trace,
    Debug,
    Info,
    Warning,
    Error,
    Fatal,
    Off,
};

struct LogMessage {
    LogLevel                              severity;
    std::uint32_t                         threadID;
    std::chrono::system_clock::time_point time;
    StringView                            message;
};

} // namespace ink

template <>
struct fmt::formatter<ink::LogLevel, char> : fmt::formatter<fmt::string_view, char> {
    template <typename FormatContext>
    auto format(ink::LogLevel level, FormatContext &ctx) const -> decltype(ctx.out()) {
        using Super = fmt::formatter<fmt::string_view, char>;
        switch (level) {
        case ink::LogLevel::Trace:
            return this->Super::format("TRACE", ctx);
        case ink::LogLevel::Debug:
            return this->Super::format("DEBUG", ctx);
        case ink::LogLevel::Info:
            return this->Super::format("INFO", ctx);
        case ink::LogLevel::Warning:
            return this->Super::format("WARNING", ctx);
        case ink::LogLevel::Error:
            return this->Super::format("ERROR", ctx);
        case ink::LogLevel::Fatal:
            return this->Super::format("FATAL", ctx);
        default:
            return this->Super::format("WTF", ctx);
        }
    }
};

template <>
struct fmt::formatter<ink::LogLevel, char16_t> : fmt::formatter<ink::StringView, char16_t> {
    template <typename FormatContext>
    auto format(ink::LogLevel level, FormatContext &ctx) const -> decltype(ctx.out()) {
        using Super = fmt::formatter<ink::StringView, char16_t>;
        switch (level) {
        case ink::LogLevel::Trace:
            return this->Super::format(u"TRACE", ctx);
        case ink::LogLevel::Debug:
            return this->Super::format(u"DEBUG", ctx);
        case ink::LogLevel::Info:
            return this->Super::format(u"INFO", ctx);
        case ink::LogLevel::Warning:
            return this->Super::format(u"WARNING", ctx);
        case ink::LogLevel::Error:
            return this->Super::format(u"ERROR", ctx);
        case ink::LogLevel::Fatal:
            return this->Super::format(u"FATAL", ctx);
        default:
            return this->Super::format(u"WTF", ctx);
        }
    }
};

template <>
struct fmt::formatter<ink::LogMessage, char16_t> : fmt::formatter<ink::StringView, char16_t> {
    template <typename FormatContext>
    auto format(const ink::LogMessage &msg, FormatContext &ctx) const -> decltype(ctx.out()) {
        return ink::formatTo(ctx.out(), u"[{}] [{}] [{}] {}", msg.threadID, msg.time, msg.severity,
                             msg.message);
    }
};

namespace ink {

class LogMessageHandler {
public:
    /// @brief
    ///   Create a new log message handler.
    ///
    /// @param level
    ///   Message severity filter level. Default filter level is @p LogLevel::Trace.
    LogMessageHandler(LogLevel level = LogLevel::Trace) noexcept : m_logLevel(level) {}

    /// @brief
    ///   Copy constructor of log message handler is disabled.
    LogMessageHandler(const LogMessageHandler &) = delete;

    /// @brief
    ///   Copy assignment of log message handler is disabled.
    auto operator=(const LogMessageHandler &) = delete;

    /// @brief
    ///   Destroy this log message handler. Destructor should never throw exceptions.
    virtual ~LogMessageHandler() noexcept = default;

    /// @brief
    ///   Pass a log message to this log message handler and handle.
    ///
    /// @param message
    ///   The log message to be handled.
    virtual auto write(const LogMessage &message) const -> void = 0;

    /// @brief
    ///   Flush current log message handler. For file log recorders, you may persistant data to hard
    ///   disk during flushing.
    virtual auto flush() -> void = 0;

    /// @brief
    ///   Get message severity filter level.
    ///
    /// @return
    ///   Message severity filter level of this log message handler.
    [[nodiscard]]
    auto logLevel() const noexcept -> LogLevel {
        return m_logLevel;
    }

    /// @brief
    ///   Set a new message severity filter level for this log message handler.
    ///
    /// @param level
    ///   The new message severity filter level to be set.
    auto logLevel(LogLevel level) noexcept -> void {
        m_logLevel = level;
    }

private:
    /// @brief
    ///   Message severity filter level.
    LogLevel m_logLevel;
};

class VisualStudioDebugLogMessageHandler final : public LogMessageHandler {
public:
    /// @brief
    ///   Create a new visual studio debug log message handler.
    ///
    /// @param level
    ///   Message severity filter level. Default filter level is @p LogLevel::Trace.
    VisualStudioDebugLogMessageHandler(LogLevel level = LogLevel::Trace) noexcept;

    /// @brief
    ///   Destroy this visual studio debug log message handler.
    ~VisualStudioDebugLogMessageHandler() noexcept override;

    /// @brief
    ///   Write the specified message to visual studio debug console.
    ///
    /// @param message
    ///   The log message to be written.
    auto write(const LogMessage &message) const -> void override;

    /// @brief
    ///   Flush visual studio debug writter.
    /// @remark
    ///   Actually this method does nothing.
    auto flush() -> void override;

private:
    /// @brief
    ///   Specifies whether visual studio debug console presents.
    const bool m_debugConsolePresent;
};

class ConsoleLogMessageHandler final : public LogMessageHandler {
public:
    /// @brief
    ///   Create a new console log message handler.
    ///
    /// @param level
    ///   Message severity filter level. Default filter level is @p LogLevel::Trace.
    ConsoleLogMessageHandler(LogLevel level = LogLevel::Trace) noexcept;

    /// @brief
    ///   Destroy this console log message handler.
    ~ConsoleLogMessageHandler() noexcept override;

    /// @brief
    ///   Write the specified message to stdout.
    ///
    /// @param message
    ///   The log message to be written.
    auto write(const LogMessage &message) const -> void override;

    /// @brief
    ///   Flush stdout.
    auto flush() -> void override;

private:
    /// @brief
    ///   std output console handle.
    void *const m_stdout;
};

class Logger {
public:
    /// @brief
    ///   Create a new logger without any message handler.
    ///
    /// @param level
    ///   Message severity filter level of this logger. Default log level is @p LogLevel::Info.
    /// @param flushLv
    ///   Flush filter level of this logger. Default flush level is @p LogLevel::Fatal.
    Logger(LogLevel level = LogLevel::Info, LogLevel flushLv = LogLevel::Fatal) noexcept;

    /// @brief
    ///   Copy constructor of logger is disabled.
    Logger(const Logger &) = delete;

    /// @brief
    ///   Copy assignment of logger is disabled.
    auto operator=(const Logger &) = delete;

    /// @brief
    ///   Write a new log message to this logger.
    /// @note
    ///   This method itself does not throw any exception, but log message handlers may throw
    ///   exceptions.
    ///
    /// @param severity
    ///   Severity level of the log message.
    /// @param message
    ///   The log message to be written.
    auto write(LogLevel severity, StringView message) const -> void;

    /// @brief
    ///   Format and write a log message to this logger.
    /// @note
    ///   This method itself does not throw any exception, but formatter and log message handlers
    ///   may throw exceptions.
    ///
    /// @tparam ...Args
    ///   Types of arguments to be formatted.
    ///
    /// @param severity
    ///   Severity level of the log message.
    /// @param fmt
    ///   Format pattern string.
    /// @param ...args
    ///   Arguments to be formatted.
    template <typename... Args>
    auto write(LogLevel severity, FormatString<Args...> fmt, Args &&...args) const -> void {
        if (severity < m_logLevel.load(std::memory_order_relaxed))
            return;

        FormatMemoryBuffer buffer;
        formatTo(std::back_inserter(buffer), fmt, std::forward<Args>(args)...);
        this->write(severity, StringView(buffer.data(), buffer.size()));
    }

    /// @brief
    ///   Create and add a new message handler to this logger.
    /// @note
    ///   This method is not thread-safe. It is OK to add the same type of message handlers for
    ///   multiple times.
    ///
    /// @tparam Handler
    ///   Type of the message handler to be added. The message handler must inherit from @p
    ///   ink::LogMessageHandler.
    /// @tparam ...Args
    ///   Types of arguments to create the message handler.
    ///
    /// @param ...args
    ///   Arguments to create the new message handler.
    ///
    /// @return
    ///   Reference to the new message handler.
    template <typename Handler,
              typename... Args,
              class = std::enable_if_t<std::is_base_of<LogMessageHandler, Handler>::value>>
    auto addMessageHandler(Args &&...args) -> Handler & {
        auto     handler = std::make_unique<Handler>(std::forward<Args>(args)...);
        Handler *ptr     = handler.get();

        m_messageHandlers.push_back(std::move(handler));
        return *ptr;
    }

    /// @brief
    ///   Write a trace log message to this logger.
    /// @note
    ///   This method itself does not throw any exception, but log message handlers may throw
    ///   exceptions.
    ///
    /// @param message
    ///   The log message to be written.
    auto trace(StringView message) const -> void {
        this->write(LogLevel::Trace, message);
    }

    /// @brief
    ///   Format and write a trace log message to this logger.
    /// @note
    ///   This method itself does not throw any exception, but formatter and log message handlers
    ///   may throw exceptions.
    ///
    /// @tparam ...Args
    ///   Types of arguments to be formatted.
    ///
    /// @param format
    ///   The format pattern string.
    /// @param ...args
    ///   Arguments to be formatted.
    template <typename... Args>
    auto trace(FormatString<Args...> format, Args &&...args) const -> void {
        this->write(LogLevel::Trace, format, std::forward<Args>(args)...);
    }

    /// @brief
    ///   Write a debug log message to this logger.
    /// @note
    ///   This method itself does not throw any exception, but log message handlers may throw
    ///   exceptions.
    ///
    /// @param message
    ///   The log message to be written.
    auto debug(StringView message) const -> void {
        this->write(LogLevel::Debug, message);
    }

    /// @brief
    ///   Format and write a debug log message to this logger.
    /// @note
    ///   This method itself does not throw any exception, but formatter and log message handlers
    ///   may throw exceptions.
    ///
    /// @tparam ...Args
    ///   Types of arguments to be formatted.
    ///
    /// @param format
    ///   The format pattern string.
    /// @param ...args
    ///   Arguments to be formatted.
    template <typename... Args>
    auto debug(FormatString<Args...> format, Args &&...args) const -> void {
        this->write(LogLevel::Debug, format, std::forward<Args>(args)...);
    }

    /// @brief
    ///   Write a info log message to this logger.
    /// @note
    ///   This method itself does not throw any exception, but log message handlers may throw
    ///   exceptions.
    ///
    /// @param message
    ///   The log message to be written.
    auto info(StringView message) const -> void {
        this->write(LogLevel::Info, message);
    }

    /// @brief
    ///   Format and write a info log message to this logger.
    /// @note
    ///   This method itself does not throw any exception, but formatter and log message handlers
    ///   may throw exceptions.
    ///
    /// @tparam ...Args
    ///   Types of arguments to be formatted.
    ///
    /// @param format
    ///   The format pattern string.
    /// @param ...args
    ///   Arguments to be formatted.
    template <typename... Args>
    auto info(FormatString<Args...> format, Args &&...args) const -> void {
        this->write(LogLevel::Info, format, std::forward<Args>(args)...);
    }

    /// @brief
    ///   Write a warning log message to this logger.
    /// @note
    ///   This method itself does not throw any exception, but log message handlers may throw
    ///   exceptions.
    ///
    /// @param message
    ///   The log message to be written.
    auto warning(StringView message) const -> void {
        this->write(LogLevel::Warning, message);
    }

    /// @brief
    ///   Format and write a warning log message to this logger.
    /// @note
    ///   This method itself does not throw any exception, but formatter and log message handlers
    ///   may throw exceptions.
    ///
    /// @tparam ...Args
    ///   Types of arguments to be formatted.
    ///
    /// @param format
    ///   The format pattern string.
    /// @param ...args
    ///   Arguments to be formatted.
    template <typename... Args>
    auto warning(FormatString<Args...> format, Args &&...args) const -> void {
        this->write(LogLevel::Warning, format, std::forward<Args>(args)...);
    }

    /// @brief
    ///   Write an error log message to this logger.
    /// @note
    ///   This method itself does not throw any exception, but log message handlers may throw
    ///   exceptions.
    ///
    /// @param message
    ///   The log message to be written.
    auto error(StringView message) const -> void {
        this->write(LogLevel::Error, message);
    }

    /// @brief
    ///   Format and write an error log message to this logger.
    /// @note
    ///   This method itself does not throw any exception, but formatter and log message handlers
    ///   may throw exceptions.
    ///
    /// @tparam ...Args
    ///   Types of arguments to be formatted.
    ///
    /// @param format
    ///   The format pattern string.
    /// @param ...args
    ///   Arguments to be formatted.
    template <typename... Args>
    auto error(FormatString<Args...> format, Args &&...args) const -> void {
        this->write(LogLevel::Error, format, std::forward<Args>(args)...);
    }

    /// @brief
    ///   Write a fatal log message to this logger.
    /// @note
    ///   This method itself does not throw any exception, but log message handlers may throw
    ///   exceptions.
    ///
    /// @param message
    ///   The log message to be written.
    auto fatal(StringView message) const -> void {
        this->write(LogLevel::Fatal, message);
    }

    /// @brief
    ///   Format and write a fatal log message to this logger.
    /// @note
    ///   This method itself does not throw any exception, but formatter and log message handlers
    ///   may throw exceptions.
    ///
    /// @tparam ...Args
    ///   Types of arguments to be formatted.
    ///
    /// @param format
    ///   The format pattern string.
    /// @param ...args
    ///   Arguments to be formatted.
    template <typename... Args>
    auto fatal(FormatString<Args...> format, Args &&...args) const -> void {
        this->write(LogLevel::Fatal, format, std::forward<Args>(args)...);
    }

    /// @brief
    ///   Get message severity filter level of this logger.
    ///
    /// @return
    ///   Message severity filter level of this logger.
    [[nodiscard]]
    auto logLevel() const noexcept -> LogLevel {
        return m_logLevel.load(std::memory_order_relaxed);
    }

    /// @brief
    ///   Set a new message severity filter level for this logger.
    ///
    /// @param level
    ///   The new message severity filter level for this logger.
    auto logLevel(LogLevel level) noexcept -> void {
        m_logLevel.store(level, std::memory_order_relaxed);
    }

    /// @brief
    ///   Get flush level of this logger.
    ///
    /// @return
    ///   Flush level of this logger.
    [[nodiscard]]
    auto flushLevel() const noexcept -> LogLevel {
        return m_flushLevel.load(std::memory_order_relaxed);
    }

    /// @brief
    ///   Set a new flush level for this logger.
    ///
    /// @param level
    ///   The new flush level for this logger.
    auto flushLevel(LogLevel level) noexcept -> void {
        m_flushLevel.store(level, std::memory_order_relaxed);
    }

    /// @brief
    ///   Flush all message handlers of this logger.
    /// @note
    ///   This method itself does not throw any exception, but message handlers may throw exceptions
    ///   while flushing.
    auto flush() -> void;

    /// @brief
    ///   Get logger singleton instance. This is the default logger of global logging functions.
    ///
    /// @return
    ///   Reference to the logger singleton instance.
    [[nodiscard]]
    static auto singleton() noexcept -> Logger &;

private:
    /// @brief
    ///   Log message handler list.
    std::vector<std::unique_ptr<LogMessageHandler>> m_messageHandlers;

    /// @brief
    ///   Message severity filter level of this logger.
    std::atomic<LogLevel> m_logLevel;

    /// @brief
    ///   Flush filter level of this logger.
    std::atomic<LogLevel> m_flushLevel;
};

/// @brief
///   Create and add a new log message handler to default logger.
///
/// @note
///   This function is not thread-safe. It is OK to add the same type of message handlers for
///   multiple times.
///
/// @tparam Handler
///   Type of the message handler to be added. The message handler must inherit from @p
///   ink::LogMessageHandler.
/// @tparam ...Args
///   Types of arguments to create the message handler.
///
/// @param ...args
///   Arguments to create the new message handler.
///
/// @return
///   Reference to the new message handler.
template <typename Handler,
          typename... Args,
          class = std::enable_if_t<std::is_base_of<LogMessageHandler, Handler>::value>>
auto addLogMessageHandler(Args &&...args) -> Handler & {
    return Logger::singleton().addMessageHandler<Handler>(std::forward<Args>(args)...);
}

/// @brief
///   Flush all message handlers of default logger.
/// @note
///   This function itself does not throw any exception, but message handlers may throw exceptions
///   while flushing.
inline auto flushLogger() -> void {
    Logger::singleton().flush();
}

/// @brief
///   Write a trace log message to default logger.
/// @note
///   This function itself does not throw any exception, but log message handlers may throw
///   exceptions.
///
/// @param message
///   The log message to be written.
inline auto logTrace(StringView message) -> void {
    Logger::singleton().trace(message);
}

/// @brief
///   Format and write a trace log message to default logger.
/// @note
///   This function itself does not throw any exception, but formatter and log message handlers
///   may throw exceptions.
///
/// @tparam ...Args
///   Types of arguments to be formatted.
///
/// @param format
///   The format pattern string.
/// @param ...args
///   Arguments to be formatted.
template <typename... Args>
auto logTrace(FormatString<Args...> format, Args &&...args) -> void {
    Logger::singleton().trace(format, std::forward<Args>(args)...);
}

/// @brief
///   Write a debug log message to default logger.
/// @note
///   This function itself does not throw any exception, but log message handlers may throw
///   exceptions.
///
/// @param message
///   The log message to be written.
inline auto logDebug(StringView message) -> void {
    Logger::singleton().debug(message);
}

/// @brief
///   Format and write a debug log message to default logger.
/// @note
///   This function itself does not throw any exception, but formatter and log message handlers
///   may throw exceptions.
///
/// @tparam ...Args
///   Types of arguments to be formatted.
///
/// @param format
///   The format pattern string.
/// @param ...args
///   Arguments to be formatted.
template <typename... Args>
auto logDebug(FormatString<Args...> format, Args &&...args) -> void {
    Logger::singleton().debug(format, std::forward<Args>(args)...);
}

/// @brief
///   Write a info log message to default logger.
/// @note
///   This function itself does not throw any exception, but log message handlers may throw
///   exceptions.
///
/// @param message
///   The log message to be written.
inline auto logInfo(StringView message) -> void {
    Logger::singleton().info(message);
}

/// @brief
///   Format and write a info log message to default logger.
/// @note
///   This function itself does not throw any exception, but formatter and log message handlers
///   may throw exceptions.
///
/// @tparam ...Args
///   Types of arguments to be formatted.
///
/// @param format
///   The format pattern string.
/// @param ...args
///   Arguments to be formatted.
template <typename... Args>
auto logInfo(FormatString<Args...> format, Args &&...args) -> void {
    Logger::singleton().info(format, std::forward<Args>(args)...);
}

/// @brief
///   Write a warning log message to default logger.
/// @note
///   This function itself does not throw any exception, but log message handlers may throw
///   exceptions.
///
/// @param message
///   The log message to be written.
inline auto logWarning(StringView message) -> void {
    Logger::singleton().warning(message);
}

/// @brief
///   Format and write a warning log message to default logger.
/// @note
///   This function itself does not throw any exception, but formatter and log message handlers
///   may throw exceptions.
///
/// @tparam ...Args
///   Types of arguments to be formatted.
///
/// @param format
///   The format pattern string.
/// @param ...args
///   Arguments to be formatted.
template <typename... Args>
auto logWarning(FormatString<Args...> format, Args &&...args) -> void {
    Logger::singleton().warning(format, std::forward<Args>(args)...);
}

/// @brief
///   Write an error log message to default logger.
/// @note
///   This function itself does not throw any exception, but log message handlers may throw
///   exceptions.
///
/// @param message
///   The log message to be written.
inline auto logError(StringView message) -> void {
    Logger::singleton().error(message);
}

/// @brief
///   Format and write an error log message to default logger.
/// @note
///   This function itself does not throw any exception, but formatter and log message handlers
///   may throw exceptions.
///
/// @tparam ...Args
///   Types of arguments to be formatted.
///
/// @param format
///   The format pattern string.
/// @param ...args
///   Arguments to be formatted.
template <typename... Args>
auto logError(FormatString<Args...> format, Args &&...args) -> void {
    Logger::singleton().error(format, std::forward<Args>(args)...);
}

/// @brief
///   Write a fatal log message to default logger.
/// @note
///   This function itself does not throw any exception, but log message handlers may throw
///   exceptions.
///
/// @param message
///   The log message to be written.
inline auto logFatal(StringView message) -> void {
    Logger::singleton().fatal(message);
}

/// @brief
///   Format and write a fatal log message to default logger.
/// @note
///   This function itself does not throw any exception, but formatter and log message handlers
///   may throw exceptions.
///
/// @tparam ...Args
///   Types of arguments to be formatted.
///
/// @param format
///   The format pattern string.
/// @param ...args
///   Arguments to be formatted.
template <typename... Args>
auto logFatal(FormatString<Args...> format, Args &&...args) -> void {
    Logger::singleton().fatal(format, std::forward<Args>(args)...);
}

} // namespace ink

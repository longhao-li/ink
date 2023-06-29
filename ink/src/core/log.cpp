#include "ink/core/log.h"

#include <Windows.h>

using namespace ink;

ink::VisualStudioDebugLogMessageHandler::VisualStudioDebugLogMessageHandler(LogLevel level) noexcept
    : LogMessageHandler(level), m_debugConsolePresent(IsDebuggerPresent()) {}

ink::VisualStudioDebugLogMessageHandler::~VisualStudioDebugLogMessageHandler() noexcept {}

auto ink::VisualStudioDebugLogMessageHandler::write(const LogMessage &message) const -> void {
    if (!m_debugConsolePresent)
        return;

    FormatMemoryBuffer buffer;
    formatTo(std::back_inserter(buffer), u"{}\n", message);
    OutputDebugStringW(reinterpret_cast<LPCWSTR>(buffer.data()));
}

auto ink::VisualStudioDebugLogMessageHandler::flush() -> void {}

ink::ConsoleLogMessageHandler::ConsoleLogMessageHandler(LogLevel level) noexcept
    : LogMessageHandler(level), m_stdout(GetStdHandle(STD_OUTPUT_HANDLE)) {}

ink::ConsoleLogMessageHandler::~ConsoleLogMessageHandler() noexcept {
    // Do not call flush() directly in destructor because it is a virtual method.
    if (m_stdout == nullptr || m_stdout == INVALID_HANDLE_VALUE)
        return;

    FlushFileBuffers(m_stdout);
}

auto ink::ConsoleLogMessageHandler::write(const LogMessage &message) const -> void {
    if (m_stdout == nullptr || m_stdout == INVALID_HANDLE_VALUE)
        return;

    FormatMemoryBuffer buffer;
    formatTo(std::back_inserter(buffer), u"{}\n", message);

    WriteConsoleW(m_stdout, reinterpret_cast<LPCWSTR>(buffer.data()),
                  static_cast<DWORD>(buffer.size()), nullptr, nullptr);
}

auto ink::ConsoleLogMessageHandler::flush() -> void {
    if (m_stdout == nullptr || m_stdout == INVALID_HANDLE_VALUE)
        return;

    FlushFileBuffers(m_stdout);
}

ink::Logger::Logger(LogLevel level, LogLevel flushLv) noexcept
    : m_messageHandlers(), m_logLevel(level), m_flushLevel(flushLv) {}

auto ink::Logger::write(LogLevel severity, StringView message) const -> void {
    if (severity < logLevel())
        return;

    static thread_local const DWORD threadID = GetCurrentThreadId();

    const LogMessage logMessage{
        /* severity = */ severity,
        /* threadID = */ static_cast<std::uint32_t>(threadID),
        /* time     = */ std::chrono::system_clock::now(),
        /* message  = */ message,
    };

    if (severity >= flushLevel()) {
        for (const auto &handler : m_messageHandlers) {
            if (severity >= handler->logLevel())
                handler->write(logMessage);

            handler->flush();
        }
    } else {
        for (const auto &handler : m_messageHandlers) {
            if (severity >= handler->logLevel())
                handler->write(logMessage);
        }
    }
}

auto ink::Logger::flush() -> void {
    for (const auto &handler : m_messageHandlers)
        handler->flush();
}

auto ink::Logger::singleton() noexcept -> Logger & {
    static Logger instance;
    return instance;
}

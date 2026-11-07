#include "edge_sync_base/logger.hpp"

#include <chrono>
#include <cstdio>
#include <ctime>

namespace edge_sync::base {
namespace {
const char* level_name(LogLevel level) {
    switch (level) {
        case LogLevel::Trace:
            return "TRACE";
        case LogLevel::Debug:
            return "DEBUG";
        case LogLevel::Info:
            return "INFO";
        case LogLevel::Warn:
            return "WARN";
        case LogLevel::Error:
            return "ERROR";
        case LogLevel::Critical:
            return "CRITICAL";
    }
    return "?";
}
}  // namespace

void ConsoleLogger::log(LogLevel level, const std::string& message) {
    if (level < min_level_)
        return;

    const auto now = std::chrono::system_clock::now();
    const auto t = std::chrono::system_clock::to_time_t(now);
    std::tm tm{};
    localtime_r(&t, &tm);

    char ts[32];
    std::strftime(ts, sizeof(ts), "%Y-%m-%d %H:%M:%S", &tm);
    std::fprintf(stderr, "%s [%s] %s\n", ts, level_name(level), message.c_str());
}

}  // namespace edge_sync::base

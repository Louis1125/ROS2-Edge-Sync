#pragma once

#include <string>

namespace edge_sync::base {

enum class LogLevel { Trace, Debug, Info, Warn, Error, Critical };

class Logger {
public:
    virtual ~Logger() = default;
    virtual void log(LogLevel level, const std::string& message) = 0;
};

// Synchronous stderr logger with level filtering. Instances are cheap; create
// one per runtime and pass it by reference/pointer into components.
class ConsoleLogger final : public Logger {
public:
    explicit ConsoleLogger(LogLevel min_level = LogLevel::Info) : min_level_(min_level) {}

    void log(LogLevel level, const std::string& message) override;

private:
    LogLevel min_level_;
};

}  // namespace edge_sync::base

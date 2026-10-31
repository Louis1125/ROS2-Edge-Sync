#pragma once

#include <string>

namespace edge_sync::base {

enum class LogLevel { Trace, Debug, Info, Warn, Error, Critical };

class Logger {
public:
    virtual ~Logger() = default;
    virtual void log(LogLevel level, const std::string& message) = 0;
};

class NullLogger final : public Logger {
public:
    void log(LogLevel, const std::string&) override {}
};

}  // namespace edge_sync::base

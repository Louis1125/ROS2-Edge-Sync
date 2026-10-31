#pragma once

#include <mutex>
#include <string>
#include <vector>

#include "edge_sync_base/logger.hpp"

namespace edge_sync::test_support {

// Records log lines for assertions in tests. Thread-safe: log() may be called
// from I/O threads while tests read the buffer.
class CapturingLogger final : public edge_sync::base::Logger {
public:
    void log(edge_sync::base::LogLevel, const std::string& message) override {
        std::lock_guard lk(mu_);
        messages_.push_back(message);
    }

    [[nodiscard]] bool contains(const std::string& needle) const {
        std::lock_guard lk(mu_);
        for (const auto& m : messages_) {
            if (m.find(needle) != std::string::npos) return true;
        }
        return false;
    }

    [[nodiscard]] std::size_t size() const {
        std::lock_guard lk(mu_);
        return messages_.size();
    }

private:
    mutable std::mutex mu_;
    std::vector<std::string> messages_;
};

}  // namespace edge_sync::test_support

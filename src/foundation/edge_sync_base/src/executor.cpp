#include "edge_sync_base/executor.hpp"

namespace edge_sync::base {

Executor::Executor(std::size_t thread_count) : thread_count_(thread_count) {}

Executor::~Executor() { stop(); }

void Executor::start() {
    for (std::size_t i = 0; i < thread_count_; ++i) {
        threads_.emplace_back([](std::stop_token) { /* TODO: drain scheduled tasks */ });
    }
}

void Executor::stop() {
    for (auto& t : threads_) t.request_stop();
    threads_.clear();
}

}  // namespace edge_sync::base

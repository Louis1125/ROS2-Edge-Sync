#pragma once

#include <cstddef>
#include <thread>
#include <vector>

namespace edge_sync::base {

class Executor {
public:
    explicit Executor(std::size_t thread_count = 1);
    ~Executor();

    Executor(const Executor&) = delete;
    Executor& operator=(const Executor&) = delete;

    void start();
    void stop();

private:
    std::size_t thread_count_;
    std::vector<std::jthread> threads_;
};

}  // namespace edge_sync::base

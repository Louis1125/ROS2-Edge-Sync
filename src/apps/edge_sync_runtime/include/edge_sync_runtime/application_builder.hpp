#pragma once

#include <memory>

#include "edge_sync_base/result.hpp"

namespace edge_sync::runtime {

class Application {
public:
    void run();
};

class ApplicationBuilder {
public:
    edge_sync::base::Result<std::unique_ptr<Application>> build(int argc, char** argv);
};

}  // namespace edge_sync::runtime

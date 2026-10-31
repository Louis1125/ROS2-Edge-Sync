#pragma once

#include <string>

#include "edge_sync_base/result.hpp"

namespace edge_sync::transport {

class TlsContext {
public:
    edge_sync::base::Result<void> load(const std::string& cert_path,
                                       const std::string& key_path);
};

}  // namespace edge_sync::transport

#include "edge_sync_base/config.hpp"

#include <fstream>

namespace edge_sync::base {

Result<Config> Config::load_file(const std::string& path) {
    std::ifstream in(path);
    if (!in) {
        return Error{ErrorCode::InvalidArgument, "cannot open config file: " + path};
    }

    Config config;
    // Minimal "key: value" parser. Replace with a full YAML loader in M1.
    std::string line;
    while (std::getline(in, line)) {
        const auto pos = line.find(':');
        if (pos == std::string::npos) continue;
        config.values_[line.substr(0, pos)] = line.substr(pos + 1);
    }
    return config;
}

bool Config::has(const std::string& key) const noexcept { return values_.count(key) > 0; }

std::string Config::get_string(const std::string& key, const std::string& fallback) const {
    const auto it = values_.find(key);
    return it != values_.end() ? it->second : fallback;
}

}  // namespace edge_sync::base

#pragma once

#include <string>
#include <unordered_map>

#include "edge_sync_base/result.hpp"

namespace edge_sync::base {

class Config {
public:
    static Result<Config> load_file(const std::string& path);

    [[nodiscard]] bool has(const std::string& key) const noexcept;
    [[nodiscard]] std::string get_string(const std::string& key,
                                         const std::string& fallback = "") const;

private:
    std::unordered_map<std::string, std::string> values_;
};

}  // namespace edge_sync::base

#pragma once

#include <string>
#include <unordered_map>

#include "edge_sync_base/result.hpp"

namespace edge_sync::base {

// Minimal indentation-based YAML subset: "key: value" lines, comments, and
// arbitrarily nested sections. Nested keys are flattened with '.' separators
// (e.g. "connection.retry.max_attempts"). Values are stored as strings and
// accessed through typed getters. This avoids a YAML dependency while keeping
// human-editable config files.
class Config {
   public:
    static Result<Config> load_file(const std::string& path);

    [[nodiscard]] bool has(const std::string& key) const noexcept;

    [[nodiscard]] std::string get_string(const std::string& key,
                                         const std::string& fallback = "") const;
    [[nodiscard]] int get_int(const std::string& key, int fallback = 0) const;
    [[nodiscard]] bool get_bool(const std::string& key, bool fallback = false) const;

   private:
    std::unordered_map<std::string, std::string> values_;
};

}  // namespace edge_sync::base

#include "edge_sync_base/config.hpp"

#include <cstdlib>
#include <fstream>
#include <string>
#include <vector>

namespace edge_sync::base {
namespace {

std::string trim(const std::string& s) {
    const auto first = s.find_first_not_of(" \t\r\n");
    if (first == std::string::npos) return {};
    const auto last = s.find_last_not_of(" \t\r\n");
    return s.substr(first, last - first + 1);
}

}  // namespace

Result<Config> Config::load_file(const std::string& path) {
    std::ifstream in(path);
    if (!in) {
        return Error{ErrorCode::InvalidArgument, "cannot open config file: " + path};
    }

    Config config;
    std::vector<std::pair<int, std::string>> sections;  // (indent, name)

    std::string line;
    int line_no = 0;
    while (std::getline(in, line)) {
        ++line_no;
        const std::string trimmed = trim(line);
        if (trimmed.empty() || trimmed[0] == '#') continue;

        int indent = 0;
        while (indent < static_cast<int>(line.size()) && line[indent] == ' ') ++indent;

        const auto pos = trimmed.find(':');
        if (pos == std::string::npos) {
            return Error{ErrorCode::InvalidArgument,
                         "malformed config line " + std::to_string(line_no) + ": " + trimmed};
        }
        const std::string key = trim(trimmed.substr(0, pos));
        const std::string value = trim(trimmed.substr(pos + 1));

        // Pop section stack to the nearest shallower indentation.
        while (!sections.empty() && sections.back().first >= indent) sections.pop_back();

        std::string full;
        for (const auto& [_, name] : sections) {
            full += name;
            full += '.';
        }
        full += key;

        if (value.empty()) {
            sections.emplace_back(indent, key);  // section header
        } else {
            config.values_[full] = value;
        }
    }
    return config;
}

bool Config::has(const std::string& key) const noexcept { return values_.count(key) > 0; }

std::string Config::get_string(const std::string& key, const std::string& fallback) const {
    const auto it = values_.find(key);
    return it != values_.end() ? it->second : fallback;
}

int Config::get_int(const std::string& key, int fallback) const {
    const auto it = values_.find(key);
    if (it == values_.end()) return fallback;
    return std::strtol(it->second.c_str(), nullptr, 10);
}

bool Config::get_bool(const std::string& key, bool fallback) const {
    const auto it = values_.find(key);
    if (it == values_.end()) return fallback;
    const auto& v = it->second;
    return v == "true" || v == "1" || v == "yes";
}

}  // namespace edge_sync::base

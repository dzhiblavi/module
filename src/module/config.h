#pragma once

#include <string>
#include <unordered_map>

#include <rfl/Generic.hpp>

namespace mod {

struct ModuleConfig {
    std::string cls = {};
    std::vector<rfl::Generic> deps = {};
};

using ModulesConfig = std::unordered_map<std::string, ModuleConfig>;

}  // namespace mod

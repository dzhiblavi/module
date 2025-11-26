#pragma once

#include "module/detail/Traits.h"

#include <expected>
#include <string>
#include <unordered_map>

namespace mod {

class Storage {
 public:
    Storage() = default;

    std::expected<void, std::string> add(const std::string& name, detail::Traits* traits);
    std::optional<detail::Traits*> get(const std::string& name);

    static Storage& instance();

 private:
    std::unordered_map<std::string, detail::Traits*> traits_;
};

}  // namespace mod

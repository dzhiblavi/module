#include "module/Storage.h"

#include <format>

namespace mod {

Result<void> Storage::add(const std::string& name, detail::Traits* traits) {
    if (traits_.contains(name)) {
        return error("trait '{}' is already registered", name);
    }

    traits_.emplace(name, traits);
    return {};
}

std::optional<detail::Traits*> Storage::get(const std::string& name) {
    if (auto it = traits_.find(name); it != traits_.end()) {
        return it->second;
    }

    return std::nullopt;
}

Storage& Storage::instance() {
    static Storage storage;
    return storage;
}

}  // namespace mod

#include "module/Context.h"

namespace mod {

Context::Context(ModulesConfig config, Storage* storage)
    : config_{std::move(config)}
    , storage_{storage} {
}

std::expected<std::shared_ptr<detail::Module>, std::string> Context::getModule(
    const std::string& name) {
    if (auto it = modules_.find(name); it != modules_.end()) {
        return it->second;
    }

    return loadModule(name);
}

std::expected<std::shared_ptr<detail::Module>, std::string> Context::loadModule(
    const std::string& name) {
    using R = std::expected<std::shared_ptr<detail::Module>, std::string>;

    auto maybe_config = getConfig(name);
    if (!maybe_config) {
        return std::unexpected("module not configured");
    }
    auto config = *std::move(maybe_config);

    auto maybe_traits = storage_->get(config.cls);
    if (!maybe_traits) {
        return std::unexpected(std::format("unknown type '{}'", config.cls));
    }

    return maybe_traits.value()->create(this, config, name).and_then([&](auto mod) -> R {
        modules_.emplace(name, mod);
        return mod;
    });
}

std::optional<ModuleConfig> Context::getConfig(const std::string& name) {
    if (auto it = config_.find(name); it != config_.end()) {
        return it->second;
    }

    return std::nullopt;
}

std::expected<void, std::string> Context::loadAllModules() {
    for (auto&& [name, config] : config_) {
        if (auto e = getModule(name); !e.has_value()) {
            return std::unexpected(std::format("error while loading '{}': {}", name, e.error()));
        }
    }

    return {};
}

}  // namespace mod

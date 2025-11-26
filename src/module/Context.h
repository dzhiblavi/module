#pragma once

#include "module/Storage.h"
#include "module/config.h"
#include "module/detail/Interface.h"

#include <rfl/type_name_t.hpp>

#include <expected>

namespace mod {

class Context {
 public:
    Context(ModulesConfig config, Storage* storage);

    std::expected<void, std::string> loadAllModules();

    template <typename T>
    std::expected<std::shared_ptr<T>, std::string> getModule(const std::string& name);

 private:
    std::expected<std::shared_ptr<detail::Module>, std::string> getModule(const std::string& name);
    std::expected<std::shared_ptr<detail::Module>, std::string> loadModule(const std::string& name);
    std::optional<ModuleConfig> getConfig(const std::string& name);

    ModulesConfig config_;
    Storage* storage_;
    std::unordered_map<std::string, std::shared_ptr<detail::Module>> modules_;
};

template <typename T>
std::expected<std::shared_ptr<T>, std::string> Context::getModule(const std::string& name) {
    return getModule(name).and_then(
        [&](std::shared_ptr<detail::Module> mod) -> std::expected<std::shared_ptr<T>, std::string> {
            if (auto t = std::dynamic_pointer_cast<T>(std::move(mod))) {
                return t;
            }
            return std::unexpected(
                std::format(
                    "type mismatch for module '{}', expected '{}'",
                    name,
                    rfl::type_name_t<T>().str()));
        });
}

}  // namespace mod

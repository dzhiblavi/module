#pragma once

#include "module/config.h"
#include "module/detail/Interface.h"

#include <expected>
#include <memory>

namespace mod {

class Context;

namespace detail {

class Traits {
 public:
    virtual ~Traits() = default;

    virtual std::expected<std::shared_ptr<Module>, std::string> create(
        Context* context, const ModuleConfig& config, const std::string& name) = 0;
};

}  // namespace detail
}  // namespace mod

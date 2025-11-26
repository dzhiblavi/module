#pragma once

#include <expected>
#include <memory>
#include <string>

namespace mod::detail {

class Module {
 public:
    virtual ~Module() = default;
};

class Factory : public Module {
 public:
    virtual std::expected<std::shared_ptr<Module>, std::string> modCreate() = 0;
};

}  // namespace mod::detail

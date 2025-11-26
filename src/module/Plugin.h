#pragma once

#include <rfl/Generic.hpp>

namespace mod {

class Plugin {
 public:
    virtual ~Plugin() = default;
    virtual rfl::Generic transform(rfl::Generic param) = 0;
};

}  // namespace mod

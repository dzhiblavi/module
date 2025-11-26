#pragma once

#include "module/Storage.h"
#include "module/detail/make_traits.h"

#include <exception>
#include <string>

namespace mod::detail {

template <typename T>
struct Registerer {
    explicit Registerer(const std::string& name) {
        if (auto e = Storage::instance().add(name, makeModuleTraits<T>()); !e) {
            std::println(stderr, "failed to register type: {}", e.error());
            std::terminate();
        }
    }
};

}  // namespace mod::detail

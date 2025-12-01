#pragma once

#include "module/Storage.h"
#include "module/detail/make_traits.h"

#include <rfl/type_name_t.hpp>

namespace mod {

template <typename T>
Result<void> registerModule(Storage* storage) {
    return storage->add(rfl::type_name_t<T>().str(), detail::makeModuleTraits<T>());
}

}  // namespace mod

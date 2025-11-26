#pragma once

#include <type_traits>

namespace mod::detail {

namespace match {

template <typename C, template <typename...> typename T>
struct IsInstanceOfTemplate : std::false_type {};

template <typename... C, template <typename...> typename T>
struct IsInstanceOfTemplate<T<C...>, T> : std::true_type {};

}  // namespace match

template <typename C, template <typename...> typename T>
inline static constexpr bool IsInstanceOfTemplate = match::IsInstanceOfTemplate<C, T>::value;

template <typename C, template <typename...> typename T>
concept InstanceOfTemplate = IsInstanceOfTemplate<C, T>;

}  // namespace mod::detail

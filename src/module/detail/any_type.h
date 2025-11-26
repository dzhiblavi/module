#pragma once

#include <type_traits>

namespace mod::detail {

struct AnyType : std::false_type {
    template <typename T>
    operator T();
};

struct AnyTypeRef : std::true_type {
    template <typename T>
    operator T();

    template <typename T>
    operator T&() const;

    template <typename T>
    operator T&&() const;

    template <typename T>
    operator const T&() const;
};

template <typename Exclude>
struct AnyType1st : std::false_type {
    template <typename T>
    requires(!std::is_same_v<std::decay_t<T>, Exclude>)
    operator T();
};

template <typename Exclude>
struct AnyType1stRef : std::true_type {
    template <typename T>
    requires(!std::is_same_v<std::decay_t<T>, Exclude>)
    operator T();

    template <typename T>
    requires(!std::is_same_v<std::decay_t<T>, Exclude>)
    operator T&() const;

    template <typename T>
    requires(!std::is_same_v<std::decay_t<T>, Exclude>)
    operator T&&() const;

    template <typename T>
    requires(!std::is_same_v<std::decay_t<T>, Exclude>)
    operator const T&() const;
};

}  // namespace mod::detail

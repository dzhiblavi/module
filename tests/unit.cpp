#include <module/Context.h>
#include <module/Plugin.h>
#include <module/config.h>
#include <module/register.h>

#include <rfl/json.hpp>

#include <catch2/catch_all.hpp>

namespace mod {

TEST_CASE("module/emplace/default_constructor") {
    struct C {
        C() = default;
    };

    Context ctx({}, nullptr);
    auto mod = ctx.emplace<C>("test");
    CHECK(mod.has_value());
}

TEST_CASE("module/emplace/duplicate") {
    struct C {
        C() = default;
    };

    Context ctx({}, nullptr);
    auto m1 = ctx.emplace<C>("test");
    CHECK(m1.has_value());

    auto m2 = ctx.emplace<C>("test");
    REQUIRE(!m2.has_value());
    std::println("{}", m2.error());
}

TEST_CASE("module/getModule/module_not_configured") {
    struct C {
        C() = default;
    };

    ModulesConfig config{{
        "test",
        ModuleConfig{.cls = "C"},
    }};

    Storage storage;
    storage.add("C", detail::makeModuleTraits<C>());
    Context ctx(config, &storage);

    auto mod = ctx.getModule<C>("other");
    REQUIRE(!mod.has_value());
    std::println("{}", mod.error());
}

TEST_CASE("module/getModule/class_not_registered") {
    struct C {
        C() = default;
    };

    ModulesConfig config{{
        "test",
        ModuleConfig{.cls = "C"},
    }};

    Storage storage;
    Context ctx(config, &storage);

    auto mod = ctx.getModule<C>("test");
    REQUIRE(!mod.has_value());
    std::println("{}", mod.error());
}

TEST_CASE("module/getModule/default_constructor") {
    struct C {
        C() = default;
    };

    ModulesConfig config{{
        "test",
        ModuleConfig{.cls = "C"},
    }};

    Storage storage;
    storage.add("C", detail::makeModuleTraits<C>());
    Context ctx(config, &storage);

    auto mod = ctx.getModule<C>("test");
    CHECK(mod.has_value());
}

TEST_CASE("module/deps/values/success") {
    struct C {
        C(int x, std::vector<std::string> y) : x{x}, y{y} {}

        int x;
        std::vector<std::string> y;
    };

    std::string config_str = R"({
        "test": {
            "cls": "C",
            "deps": [
                10,
                [
                    "aba",
                    "caba"
                ]
            ]
        }
    })";

    auto config = rfl::json::read<ModulesConfig>(config_str);
    REQUIRE(config);

    Storage storage;
    storage.add("C", detail::makeModuleTraits<C>());
    Context ctx(*config, &storage);

    auto mod = ctx.getModule<C>("test");
    CHECK(mod.has_value());
    CHECK((*mod)->x == 10);
    CHECK((*mod)->y == std::vector<std::string>{"aba", "caba"});
}

TEST_CASE("module/deps/values/type_mismatch") {
    struct C {
        C(int x) : x{x} {}
        int x;
    };

    std::string config_str = R"({
        "test": {
            "cls": "C",
            "deps": ["abacaba"]
        }
    })";

    auto config = rfl::json::read<ModulesConfig>(config_str);
    REQUIRE(config);

    Storage storage;
    storage.add("C", detail::makeModuleTraits<C>());
    Context ctx(*config, &storage);

    auto mod = ctx.getModule<C>("test");
    CHECK(!mod.has_value());
    std::println("{}", mod.error());
}

TEST_CASE("module/deps/values/missing") {
    struct C {
        C(int x) : x{x} {}
        int x;
    };

    std::string config_str = R"({
        "test": {
            "cls": "C",
            "deps": []
        }
    })";

    auto config = rfl::json::read<ModulesConfig>(config_str);
    REQUIRE(config);

    Storage storage;
    storage.add("C", detail::makeModuleTraits<C>());
    Context ctx(*config, &storage);

    auto mod = ctx.getModule<C>("test");
    CHECK(!mod.has_value());
    std::println("{}", mod.error());
}

TEST_CASE("module/deps/module/pointer_types") {
    struct D {
        D() = default;
    };

    struct C {
        C(D& dr, D* dp, const D* dcp, std::shared_ptr<D> ds, std::weak_ptr<D> dw) {
            CHECK(&dr == dp);
            CHECK(&dr == dcp);
            CHECK(&dr == ds.get());
            CHECK(&dr == dw.lock().get());
        }
    };

    std::string config_str = R"({
        "dep": {
            "cls": "D"
        },
        "test": {
            "cls": "C",
            "deps": [
                "dep",
                "dep",
                "dep",
                "dep",
                "dep"
            ]
        }
    })";

    auto config = rfl::json::read<ModulesConfig, rfl::DefaultIfMissing>(config_str);
    REQUIRE(config);

    Storage storage;
    storage.add("C", detail::makeModuleTraits<C>());
    storage.add("D", detail::makeModuleTraits<D>());
    Context ctx(*config, &storage);

    auto mod = ctx.getModule<C>("test");
    CHECK(mod.has_value());
}

TEST_CASE("module/deps/module/vector") {
    struct D {
        D() = default;
    };

    struct C {
        C(std::vector<D*> modules) {
            CHECK(modules.size() == 3);
            for (auto* m : modules) {
                CHECK(m == modules.front());
            }
        }
    };

    std::string config_str = R"({
        "dep": {
            "cls": "D"
        },
        "test": {
            "cls": "C",
            "deps": [
                [
                    "dep",
                    "dep",
                    "dep"
                ]
            ]
        }
    })";

    auto config = rfl::json::read<ModulesConfig, rfl::DefaultIfMissing>(config_str);
    REQUIRE(config);

    Storage storage;
    storage.add("C", detail::makeModuleTraits<C>());
    storage.add("D", detail::makeModuleTraits<D>());
    Context ctx(*config, &storage);

    auto mod = ctx.getModule<C>("test");
    CHECK(mod.has_value());
}

TEST_CASE("module/deps/module/unordered_map") {
    struct D {
        D() = default;
    };

    struct C {
        C(std::unordered_map<D*, D*> modules) {
            CHECK(modules.size() == 2);
            for (auto&& [k, v] : modules) {
                CHECK(k != v);
            }
        }
    };

    std::string config_str = R"({
        "dep1": {
            "cls": "D"
        },
        "dep2": {
            "cls": "D"
        },
        "test": {
            "cls": "C",
            "deps": [
                {
                    "dep1": "dep2",
                    "dep2": "dep1"
                }
            ]
        }
    })";

    auto config = rfl::json::read<ModulesConfig, rfl::DefaultIfMissing>(config_str);
    REQUIRE(config);

    Storage storage;
    storage.add("C", detail::makeModuleTraits<C>());
    storage.add("D", detail::makeModuleTraits<D>());
    Context ctx(*config, &storage);

    auto mod = ctx.getModule<C>("test");
    CHECK(mod.has_value());
}

TEST_CASE("module/deps/plugin/transform_once") {
    struct M : Plugin {
        Result<rfl::Generic> transform(rfl::Generic param) override {
            auto value = (*param.to_object()).at("value").to_int().value();
            return value * 2;
        }
    };

    struct C {
        C(int x) { CHECK(x == 2 * 123); }
    };

    std::string config_str = R"({
        "clap": {
            "cls": "M"
        },
        "test": {
            "cls": "C",
            "deps": [
                {
                    "@plugin": "clap",
                    "value": 123
                }
            ]
        }
    })";

    auto config = rfl::json::read<ModulesConfig, rfl::DefaultIfMissing>(config_str);
    REQUIRE(config);

    Storage storage;
    storage.add("C", detail::makeModuleTraits<C>());
    storage.add("M", detail::makeModuleTraits<M>());
    Context ctx(*config, &storage);

    auto mod = ctx.getModule<C>("test");
    CHECK(mod.has_value());
}

TEST_CASE("module/deps/plugin/transform_recursive") {
    struct M : Plugin {
        Result<rfl::Generic> transform(rfl::Generic param) override {
            auto value = (*param.to_object()).at("value").to_int().value();
            return value * 2;
        }
    };

    struct C {
        struct Config {
            int x;
        };

        C(Config c) { CHECK(c.x == 2 * 2 * 123); }
    };

    std::string config_str = R"({
        "clap": {
            "cls": "M"
        },
        "test": {
            "cls": "C",
            "deps": [
                {
                    "x": {
                        "@plugin": "clap",
                        "value": {
                            "@plugin": "clap",
                            "value": 123
                        }
                    }
                }
            ]
        }
    })";

    auto config = rfl::json::read<ModulesConfig, rfl::DefaultIfMissing>(config_str);
    REQUIRE(config);

    Storage storage;
    storage.add("C", detail::makeModuleTraits<C>());
    storage.add("M", detail::makeModuleTraits<M>());
    Context ctx(*config, &storage);

    auto mod = ctx.getModule<C>("test");
    CHECK(mod.has_value());
}

}  // namespace mod

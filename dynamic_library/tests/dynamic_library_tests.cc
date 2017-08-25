#include <catch.hpp>
#include <leatherman/dynamic_library/dynamic_library.hpp>
#include "fixtures.hpp"

using namespace leatherman::dynamic_library;

std::string const lib_path = TEST_LIB_DIRECTORY + std::string("/libtest.so");
std::string const lib_path2 = TEST_LIB_DIRECTORY + std::string("/libtest1.so");

TEST_CASE("dynamic_library::load and dynamic_library::close", "[dyn-lib]") {
    SECTION("should not be loaded by default") {
        dynamic_library lib;
        REQUIRE_FALSE(lib.loaded());
    }

    SECTION("should load library from path, then close it"){
        dynamic_library lib;
        REQUIRE(lib.load(lib_path));
        REQUIRE(lib.loaded());
        REQUIRE(lib.first_load());
        lib.close();
        REQUIRE_FALSE(lib.loaded());
    }

    SECTION("should fail to load a nonexistent library") {
        dynamic_library lib;
        REQUIRE_FALSE(lib.load("no_such_library"));
    }
}

TEST_CASE("dynamic_library::find_symbol", "[dyn-lib]"){
    dynamic_library lib;
    REQUIRE(lib.load(lib_path));

    SECTION("should fail to find nonexistent symbol") {
        REQUIRE_FALSE(lib.find_symbol("not_here"));
        REQUIRE_THROWS(lib.find_symbol("not_here", true));
    }

    SECTION("should find library function") {
        REQUIRE(lib.find_symbol("hello"));
    }

    SECTION("should find aliased symbol"){
        dynamic_library lib2;
        REQUIRE(lib2.load(lib_path2));
        REQUIRE(lib2.find_symbol("not_here", false, "goodbye"));
    }
}

TEST_CASE("dynamic_library::dyanmic_library(dynamic_library && other)", "[dyn-lib]") {
    SECTION("should move library to new variable") {
        dynamic_library lib;
        REQUIRE(lib.load(lib_path));
        REQUIRE(lib.loaded());
        dynamic_library lib2 = std::move(lib);
        REQUIRE(lib2.loaded());
        REQUIRE(lib2.name() == lib_path);
        REQUIRE_FALSE(lib.loaded());
    }
}

#ifdef _WIN32
TEST_CASE("dynamic_library::find_by_pattern", "[dyn-lib]"){
    SECTION("should fail to find a missing library"){
        REQUIRE_FALSE(dynamic_library::find_by_pattern("libtest1").loaded());
    }

    SECTION("should find a library matching a pattern"){
        dynamic_library lib;
        lib.load(lib_path);
        REQUIRE_FALSE(dynamic_library::find_by_pattern("libtest1").loaded());
        dynamic_library lib2;
        lib2.load(lib_path2);
        REQUIRE(dynamic_library::find_by_pattern("libtest1").loaded());
    }
}
#else
TEST_CASE("dynamic_library::find_by_symbol", "[dyn-lib]"){
    SECTION("should fail to find a missing symbol"){
        REQUIRE_FALSE(dynamic_library::find_by_symbol("no_such_symbol").loaded());
    }

    SECTION("should find a library with the given symbol"){
        dynamic_library lib;
        REQUIRE(lib.load(lib_path));
        REQUIRE_FALSE(dynamic_library::find_by_symbol("goodbye").loaded());
        dynamic_library lib2;
        REQUIRE(lib2.load(lib_path2, true));
        REQUIRE(lib2.find_symbol("goodbye"));
        REQUIRE(dynamic_library::find_by_symbol("goodbye").loaded());
    }
}
#endif

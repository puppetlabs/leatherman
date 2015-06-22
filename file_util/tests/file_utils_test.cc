#include "test.hpp"

#include <leatherman/file_util/file.hpp>

#ifdef _WIN32
#include <windows.h>
#endif

namespace leatherman { namespace file_util {

// TODO(ale): consider making file_utils.cpp and string_utils.cpp
// consistent across cthun-agent and pegasus before writing more tests

TEST_CASE("FileUtils::tildeExpand", "[utils]") {
#ifdef _WIN32
    _putenv("USERPROFILE=/testhome");
#else
    setenv("HOME", "/testhome", 1);
#endif

    SECTION("empty path should be empty") {
        REQUIRE(tildeExpand("") == "");
    }

    SECTION("spaces should be preserved") {
        REQUIRE(tildeExpand("i like spaces") == "i like spaces");
    }

    SECTION("should expand using environment variable") {
        CHECK(tildeExpand("~") == "/testhome");
        CHECK(tildeExpand("~/") == "/testhome/");
        CHECK(tildeExpand("~/foo") == "/testhome/foo");
    }

    SECTION("only a ~ at the start") {
        REQUIRE(tildeExpand("/foo/bar~") == "/foo/bar~");
    }

    SECTION("~baz/foo does not expand") {
        REQUIRE(tildeExpand("~baz/foo") == "~baz/foo");
    }
}

TEST_CASE("shellQuote", "[utils]") {
    SECTION("empty string") {
        REQUIRE(shellQuote("") == "\"\"");
    }

    SECTION("single word") {
        REQUIRE(shellQuote("plain") == "\"plain\"");
    }

    SECTION("words separated by space") {
        REQUIRE(shellQuote("a space") == "\"a space\"");
    }

    SECTION("exclamation mark") {
        REQUIRE(shellQuote("!csh") == "\"!csh\"");
    }

    SECTION("single quote before expression") {
        REQUIRE(shellQuote("'open quote") == "\"'open quote\"");
    }

    SECTION("single quote after expression") {
        REQUIRE(shellQuote("close quote'") == "\"close quote'\"");
    }

    SECTION("double quote before expression") {
        REQUIRE(shellQuote("\"open doublequote")
                == "\"\\\"open doublequote\"");
    }

    SECTION("double quote after expression") {
        REQUIRE(shellQuote("close doublequote\"")
                == "\"close doublequote\\\"\"");
    }
}

}}  // namespace leatherman::file_util

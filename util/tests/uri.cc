#include <catch.hpp>
#include <leatherman/util/uri.hpp>

namespace lth_util = leatherman::util;

TEST_CASE("parses a uri") {
    SECTION("full uri") {
        auto uri = lth_util::uri("https://foo:1234/bar?some=1&other=2");
        REQUIRE(uri.protocol == "https");
        REQUIRE(uri.host == "foo");
        REQUIRE(uri.port == "1234");
        REQUIRE(uri.path == "/bar");
        REQUIRE(uri.query == "?some=1&other=2");
    }

    SECTION("without protocol") {
        auto uri = lth_util::uri("foo:1234/bar?some=1&other=2");
        REQUIRE(uri.protocol == "");
        REQUIRE(uri.host == "foo");
        REQUIRE(uri.port == "1234");
        REQUIRE(uri.path == "/bar");
        REQUIRE(uri.query == "?some=1&other=2");
    }

    SECTION("without host") {
        auto uri = lth_util::uri("https://:1234/bar?some=1&other=2");
        REQUIRE(uri.protocol == "https");
        REQUIRE(uri.host == "");
        REQUIRE(uri.port == "1234");
        REQUIRE(uri.path == "/bar");
        REQUIRE(uri.query == "?some=1&other=2");
    }

    SECTION("without port") {
        auto uri = lth_util::uri("https://foo/bar?some=1&other=2");
        REQUIRE(uri.protocol == "https");
        REQUIRE(uri.host == "foo");
        REQUIRE(uri.port == "");
        REQUIRE(uri.path == "/bar");
        REQUIRE(uri.query == "?some=1&other=2");
    }

    SECTION("with missing port") {
        auto uri = lth_util::uri("https://foo:/bar?some=1&other=2");
        REQUIRE(uri.protocol == "https");
        REQUIRE(uri.host == "foo");
        REQUIRE(uri.port == "");
        REQUIRE(uri.path == "/bar");
        REQUIRE(uri.query == "?some=1&other=2");
    }

    SECTION("without path") {
        auto uri = lth_util::uri("https://foo:1234?some=1&other=2");
        REQUIRE(uri.protocol == "https");
        REQUIRE(uri.host == "foo");
        REQUIRE(uri.port == "1234");
        REQUIRE(uri.path == "");
        REQUIRE(uri.query == "?some=1&other=2");
    }

    SECTION("without query") {
        auto uri = lth_util::uri("https://foo:1234/bar");
        REQUIRE(uri.protocol == "https");
        REQUIRE(uri.host == "foo");
        REQUIRE(uri.port == "1234");
        REQUIRE(uri.path == "/bar");
        REQUIRE(uri.query == "");
    }

    SECTION("only host") {
        auto uri = lth_util::uri("foo");
        REQUIRE(uri.protocol == "");
        REQUIRE(uri.host == "foo");
        REQUIRE(uri.port == "");
        REQUIRE(uri.path == "");
        REQUIRE(uri.query == "");
    }

    SECTION("protocol, host, and port") {
        auto uri = lth_util::uri("https://foo:1234");
        REQUIRE(uri.protocol == "https");
        REQUIRE(uri.host == "foo");
        REQUIRE(uri.port == "1234");
        REQUIRE(uri.path == "");
        REQUIRE(uri.query == "");
    }
}

TEST_CASE("prints a uri") {
    SECTION("full uri") {
        auto uri = lth_util::uri("https://foo:1234/bar?some=1&other=2");
        REQUIRE(uri.str() == "https://foo:1234/bar?some=1&other=2");
    }

    SECTION("host and port") {
        auto uri = lth_util::uri("foo:1234");
        REQUIRE(uri.str() == "foo:1234");
    }

    SECTION("protocol, host, and port") {
        auto uri = lth_util::uri("https://foo:1234");
        REQUIRE(uri.str() == "https://foo:1234");
    }
}

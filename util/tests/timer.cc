#include <catch.hpp>
#include <leatherman/util/timer.hpp>

using namespace leatherman::util;

SCENARIO("Using Timer", "[util]") {
    SECTION("can instantiate") {
        REQUIRE_NOTHROW(Timer());
    }

    SECTION("can reset") {
        Timer t {};
        REQUIRE_NOTHROW(t.reset());
    }

    SECTION("can retrieve durations [s]") {
        Timer t {};
        auto d1_s = t.elapsed_seconds();
        auto d2_s = t.elapsed_seconds();

        REQUIRE(d1_s <= d2_s);
    }

    SECTION("can retrieve durations [ms]") {
        Timer t {};
        auto d1_ms = t.elapsed_milliseconds();
        auto d2_ms = t.elapsed_milliseconds();

        REQUIRE(d1_ms <= d2_ms);
    }

    SECTION("can retrieve durations after resetting") {
        Timer t {};
        t.reset();

        REQUIRE_NOTHROW(t.elapsed_seconds());
        REQUIRE_NOTHROW(t.elapsed_milliseconds());
    }
}

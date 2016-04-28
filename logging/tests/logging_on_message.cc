#include <catch.hpp>
#include <leatherman/logging/logging.hpp>
#include <boost/nowide/convert.hpp>
#include "logging.hpp"

using namespace std;
using namespace leatherman::logging;

TEST_CASE("logging with on_message") {
    leatherman::test::logging_context ctx(log_level::trace);

    string message;
    log_level level;
    on_message([&](log_level lvl, string const& msg) {
        level = lvl;
        message = msg;
        return false;
    });

    SECTION("a TRACE message is logged to on_message") {
        LOG_TRACE("trace message");
        REQUIRE(level == log_level::trace);
        REQUIRE(message == "trace message");
    }
    SECTION("a DEBUG message is logged to on_message") {
        LOG_DEBUG("debug message");
        REQUIRE(level == log_level::debug);
        REQUIRE(message == "debug message");
    }
    SECTION("an INFO message is logged to on_message") {
        LOG_INFO("info message");
        REQUIRE(level == log_level::info);
        REQUIRE(message == "info message");
    }
    SECTION("a WARNING message is logged to on_message") {
        LOG_WARNING("warning message");
        REQUIRE(level == log_level::warning);
        REQUIRE(message == "warning message");
    }
    SECTION("an ERROR message is logged to on_message") {
        LOG_ERROR("error message");
        REQUIRE(level == log_level::error);
        REQUIRE(message == "error message");
    }
    SECTION("a FATAL message is logged to on_message") {
        LOG_FATAL("fatal message");
        REQUIRE(level == log_level::fatal);
        REQUIRE(message == "fatal message");
    }
#if 0
    SECTION("a unicode characters to log") {
        const wstring symbols[] = {L"\u2122", L"\u2744", L"\u039b"};
        for (auto const& s : symbols) {
            auto utf8 = boost::nowide::narrow(s);
            LOG_INFO(utf8);
            REQUIRE(level == log_level::info);
            REQUIRE(message == utf8);
        }
    }
#endif
}

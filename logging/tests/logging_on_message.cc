#include <catch.hpp>
#include <leatherman/logging/logging.hpp>
#include <boost/nowide/convert.hpp>

using namespace std;
using namespace leatherman::logging;

SCENARIO("logging with on_message") {
    logging_test_context context;
    string message;
    log_level level;
    on_message([&](log_level lvl, string const& msg) {
        level = lvl;
        message = msg;
        return false;
    });
    GIVEN("a TRACE message to log") {
        LOG_TRACE("trace message");
        THEN("on_message is called with the message") {
            REQUIRE(level == log_level::trace);
            REQUIRE(message == "trace message");
        }
    }
    GIVEN("a DEBUG message to log") {
        LOG_DEBUG("debug message");
        THEN("on_message is called with the message") {
            REQUIRE(level == log_level::debug);
            REQUIRE(message == "debug message");
        }
    }
    GIVEN("a INFO message to log") {
        LOG_INFO("info message");
        THEN("on_message is called with the message") {
            REQUIRE(level == log_level::info);
            REQUIRE(message == "info message");
        }
    }
    GIVEN("a WARNING message to log") {
        LOG_WARNING("warning message");
        THEN("on_message is called with the message") {
            REQUIRE(level == log_level::warning);
            REQUIRE(message == "warning message");
        }
    }
    GIVEN("a ERROR message to log") {
        LOG_ERROR("error message");
        THEN("on_message is called with the message") {
            REQUIRE(level == log_level::error);
            REQUIRE(message == "error message");
        }
    }
    GIVEN("a FATAL message to log") {
        LOG_FATAL("fatal message");
        THEN("on_message is called with the message") {
            REQUIRE(level == log_level::fatal);
            REQUIRE(message == "fatal message");
        }
    }
    GIVEN("a unicode characters to log") {
        const wstring symbols[] = {L"\u2122", L"\u2744", L"\u039b"};
        for (auto const& s : symbols) {
            auto utf8 = boost::nowide::narrow(s);
            LOG_INFO(utf8);
            REQUIRE(level == log_level::info);
            REQUIRE(message == utf8);
        }
    }
}

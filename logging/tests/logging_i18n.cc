#ifndef LEATHERMAN_I18N
#define LEATHERMAN_I18N
#endif

#undef PROJECT_NAME
#define PROJECT_NAME "leatherman_logging"

#include <catch.hpp>
#include <leatherman/logging/logging.hpp>
#include <boost/nowide/convert.hpp>
#include <boost/nowide/iostream.hpp>
#include <algorithm>
#include "logging.hpp"

using namespace std;
using namespace leatherman::logging;

#define _(x) x

TEST_CASE("logging i18n with on_message") {
    leatherman::locale::get_locale("fr.UTF-8", PROJECT_NAME, {PROJECT_DIR});
    leatherman::test::logging_context ctx(log_level::trace);

    string message;
    log_level level;
    on_message([&](log_level lvl, string const& msg) {
        level = lvl;
        message = msg;
        return false;
    });

    SECTION("a TRACE message to log is not translated") {
        LOG_TRACE("trace logging");
        REQUIRE(level == log_level::trace);
        REQUIRE(message == "trace logging");
    }
    SECTION("a TRACE message with substitution is not translated") {
        LOG_TRACE("trace logging is {1}", "trace");
        REQUIRE(level == log_level::trace);
        REQUIRE(message == "trace logging is trace");
    }
    SECTION("a DEBUG message to log is translated") {
        LOG_DEBUG("debug logging");
        REQUIRE(level == log_level::debug);
        REQUIRE(message == "l'enregistrement de débogage");
    }
    SECTION("a DEBUG message with substitution is translated") {
        LOG_DEBUG("debug logging is {1}", "debug");
        REQUIRE(level == log_level::debug);
        REQUIRE(message == "l'enregistrement de débogage est debug");
    }
    SECTION("a INFO message to log is translated") {
        LOG_INFO("info logging");
        REQUIRE(level == log_level::info);
        REQUIRE(message == "info exploitation forestière");
    }
    SECTION("a INFO message with substitution is translated") {
        LOG_INFO("info logging is {1}", "info");
        REQUIRE(level == log_level::info);
        REQUIRE(message == "info exploitation forestière est info");
    }
    SECTION("a WARNING message to log is translated") {
        LOG_WARNING("warning logging");
        REQUIRE(level == log_level::warning);
        REQUIRE(message == "journalisation d'avertissement");
    }
    SECTION("a WARNING message with substitution is translated") {
        LOG_WARNING("warning logging is {1}", "warning");
        REQUIRE(level == log_level::warning);
        REQUIRE(message == "journalisation d'avertissement est warning");
    }
    SECTION("a ERROR message to log is translated") {
        LOG_ERROR("error message");
        REQUIRE(level == log_level::error);
        REQUIRE(message == "message d'erreur");
    }
    SECTION("a ERROR message with substitution is translated") {
        LOG_ERROR("error message is {1}", "error");
        REQUIRE(level == log_level::error);
        REQUIRE(message == "un message d'erreur est error");
    }
    SECTION("a FATAL message to log is translated") {
        LOG_FATAL("fatal message");
        REQUIRE(level == log_level::fatal);
        REQUIRE(message == "un message fatal");
    }
    SECTION("a FATAL message with substitution is translated") {
        LOG_FATAL("fatal message is {1}", "fatal");
        REQUIRE(level == log_level::fatal);
        REQUIRE(message == "un message fatal est fatal");
    }
    SECTION("a unicode characters to log") {
        wstring symbols = _(L"\u2122\u2744\u039b");
        auto utf8 = boost::nowide::narrow(symbols);
        reverse(symbols.begin(), symbols.end());
        auto utf8_reverse = boost::nowide::narrow(symbols);
        LOG_INFO(utf8);
        REQUIRE(level == log_level::info);
        REQUIRE(message == utf8_reverse);
    }

    leatherman::locale::clear_domain();
}

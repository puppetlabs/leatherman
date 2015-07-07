#include <catch.hpp>
#include <leatherman/logging/logging.hpp>
#include <boost/log/sinks/sync_frontend.hpp>
#include <boost/log/sinks/basic_sink_backend.hpp>
#include <boost/nowide/convert.hpp>

using namespace std;
using namespace leatherman::logging;
namespace sinks = boost::log::sinks;

struct custom_log_appender :
    sinks::basic_formatted_sink_backend<char, sinks::synchronized_feeding>
{
    void consume(boost::log::record_view const& rec, string_type const& message)
    {
        stringstream s;
        s << boost::log::extract<LogLevel>("Severity", rec);
        _level = s.str();
        _message = message;
    }

    string _level;
    string _message;
};

struct logging_test_context
{
    using sink_t = sinks::synchronous_sink<custom_log_appender>;

    logging_test_context(LogLevel lvl = LogLevel::trace)
    {
        set_level(lvl);
        clear_error_logged_flag();

        colorize(_color, lvl);
        colorize(_none);

        _appender.reset(new custom_log_appender());
        _sink.reset(new sink_t(_appender));

        auto core = boost::log::core::get();
        core->add_sink(_sink);
    }

    ~logging_test_context()
    {
        set_level(LogLevel::none);
        on_message(nullptr);
        clear_error_logged_flag();

        auto core = boost::log::core::get();
        core->reset_filter();
        core->remove_sink(_sink);

        _sink.reset();
        _appender.reset();
    }

    string const& level() const
    {
        return _appender->_level;
    }

    string const& message() const
    {
        return _appender->_message;
    }

    string color() const
    {
        return _color.str();
    }

    string none() const
    {
        return _none.str();
    }

 private:
    boost::shared_ptr<custom_log_appender> _appender;
    boost::shared_ptr<sink_t> _sink;
    ostringstream _color, _none;
};

SCENARIO("logging with a TRACE level") {
    logging_test_context context(LogLevel::trace);
    REQUIRE(LOG_IS_TRACE_ENABLED());
    LOG_TRACE("testing %1% %2% %3%", 1, "2", 3.0);
    REQUIRE(context.level() == "TRACE");
    REQUIRE(context.message() == context.color() + "testing 1 2 3" + context.none());
    log("test", LogLevel::trace, 0, "testing %1% %2% %3%", 1, "2", 3.0);
    REQUIRE(context.level() == "TRACE");
    REQUIRE(context.message() == context.color() + "testing 1 2 3" + context.none());
    REQUIRE_FALSE(error_has_been_logged());
}

SCENARIO("logging with a DEBUG level") {
    logging_test_context context(LogLevel::debug);
    REQUIRE(LOG_IS_DEBUG_ENABLED());
    LOG_DEBUG("testing %1% %2% %3%", 1, "2", 3.0);
    REQUIRE(context.level() == "DEBUG");
    REQUIRE(context.message() == context.color() + "testing 1 2 3" + context.none());
    log("test", LogLevel::debug, 0, "testing %1% %2% %3%", 1, "2", 3.0);
    REQUIRE(context.level() == "DEBUG");
    REQUIRE(context.message() == context.color() + "testing 1 2 3" + context.none());
    REQUIRE_FALSE(error_has_been_logged());
}

SCENARIO("logging with an INFO level") {
    logging_test_context context(LogLevel::info);
    REQUIRE(LOG_IS_INFO_ENABLED());
    LOG_INFO("testing %1% %2% %3%", 1, "2", 3.0);
    REQUIRE(context.level() == "INFO");
    REQUIRE(context.message() == context.color() + "testing 1 2 3" + context.none());
    log("test", LogLevel::info, 0, "testing %1% %2% %3%", 1, "2", 3.0);
    REQUIRE(context.level() == "INFO");
    REQUIRE(context.message() == context.color() + "testing 1 2 3" + context.none());
    REQUIRE_FALSE(error_has_been_logged());
}

SCENARIO("logging with a WARNING level") {
    logging_test_context context(LogLevel::warning);
    REQUIRE(LOG_IS_WARNING_ENABLED());
    LOG_WARNING("testing %1% %2% %3%", 1, "2", 3.0);
    REQUIRE(context.level() == "WARN");
    REQUIRE(context.message() == context.color() + "testing 1 2 3" + context.none());
    log("test", LogLevel::warning, 0, "testing %1% %2% %3%", 1, "2", 3.0);
    REQUIRE(context.level() == "WARN");
    REQUIRE(context.message() == context.color() + "testing 1 2 3" + context.none());
    REQUIRE_FALSE(error_has_been_logged());
}

SCENARIO("logging with an ERROR level") {
    logging_test_context context(LogLevel::error);
    REQUIRE(LOG_IS_ERROR_ENABLED());
    LOG_ERROR("testing %1% %2% %3%", 1, "2", 3.0);
    REQUIRE(context.level() == "ERROR");
    REQUIRE(context.message() == context.color() + "testing 1 2 3" + context.none());
    log("test", LogLevel::error, 0, "testing %1% %2% %3%", 1, "2", 3.0);
    REQUIRE(context.level() == "ERROR");
    REQUIRE(context.message() == context.color() + "testing 1 2 3" + context.none());
    REQUIRE(error_has_been_logged());
}

SCENARIO("logging with a FATAL level") {
    logging_test_context context(LogLevel::fatal);
    REQUIRE(LOG_IS_FATAL_ENABLED());
    LOG_FATAL("testing %1% %2% %3%", 1, "2", 3.0);
    REQUIRE(context.level() == "FATAL");
    REQUIRE(context.message() == context.color() + "testing 1 2 3" + context.none());
    log("test", LogLevel::fatal, 0, "testing %1% %2% %3%", 1, "2", 3.0);
    REQUIRE(context.level() == "FATAL");
    REQUIRE(context.message() == context.color() + "testing 1 2 3" + context.none());
    REQUIRE(error_has_been_logged());
}

SCENARIO("logging with on_message") {
    logging_test_context context;
    string message;
    LogLevel level;
    on_message([&](LogLevel lvl, string const& msg) {
        level = lvl;
        message = msg;
        return false;
    });
    GIVEN("a TRACE message to log") {
        LOG_TRACE("trace message");
        THEN("on_message is called with the message") {
            REQUIRE(level == LogLevel::trace);
            REQUIRE(message == "trace message");
        }
    }
    GIVEN("a DEBUG message to log") {
        LOG_DEBUG("debug message");
        THEN("on_message is called with the message") {
            REQUIRE(level == LogLevel::debug);
            REQUIRE(message == "debug message");
        }
    }
    GIVEN("a INFO message to log") {
        LOG_INFO("info message");
        THEN("on_message is called with the message") {
            REQUIRE(level == LogLevel::info);
            REQUIRE(message == "info message");
        }
    }
    GIVEN("a WARNING message to log") {
        LOG_WARNING("warning message");
        THEN("on_message is called with the message") {
            REQUIRE(level == LogLevel::warning);
            REQUIRE(message == "warning message");
        }
    }
    GIVEN("a ERROR message to log") {
        LOG_ERROR("error message");
        THEN("on_message is called with the message") {
            REQUIRE(level == LogLevel::error);
            REQUIRE(message == "error message");
        }
    }
    GIVEN("a FATAL message to log") {
        LOG_FATAL("fatal message");
        THEN("on_message is called with the message") {
            REQUIRE(level == LogLevel::fatal);
            REQUIRE(message == "fatal message");
        }
    }
    GIVEN("a unicode characters to log") {
        const wstring symbols[] = {L"\u2122", L"\u2744", L"\u039b"};
        for (auto const& s : symbols) {
            auto utf8 = boost::nowide::narrow(s);
            LOG_INFO(utf8);
            REQUIRE(level == LogLevel::info);
            REQUIRE(message == utf8);
        }
    }
}

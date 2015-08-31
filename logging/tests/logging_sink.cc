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
        auto lvl = boost::log::extract<log_level>("Severity", rec);
        stringstream s;
        s << boost::log::extract<log_level>("Severity", rec);

        log_level read_lvl;
        s >> read_lvl;
        REQUIRE(read_lvl == lvl);

        _level = s.str();
        _message = message;
        _namespace = boost::log::extract<string>("Namespace", rec).get();
    }

    string _level;
    string _message;
    string _namespace;
};

struct logging_test_context
{
    using sink_t = sinks::synchronous_sink<custom_log_appender>;

    logging_test_context(log_level lvl = log_level::trace, string ns = "")
    {
        set_level(lvl);
        REQUIRE(get_level() == lvl);
        clear_error_logged_flag();
        if (!ns.empty()) {
            override_namespace = ns;
        }

        colorize(_color, lvl);
        colorize(_none);

        _appender.reset(new custom_log_appender());
        _sink.reset(new sink_t(_appender));

        auto core = boost::log::core::get();
        core->add_sink(_sink);
    }

    ~logging_test_context()
    {
        set_level(log_level::none);
        REQUIRE(get_level() == log_level::none);
        on_message(nullptr);
        clear_error_logged_flag();
        override_namespace = "";

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

    string const& ns() const
    {
        return _appender->_namespace;
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
    logging_test_context context(log_level::trace);
    REQUIRE(LOG_IS_TRACE_ENABLED());
    LOG_TRACE("testing %1% %2% %3%", 1, "2", 3.0);
    REQUIRE(context.level() == "TRACE");
    REQUIRE(context.message() == context.color() + "testing 1 2 3" + context.none());
    REQUIRE(context.ns() == LEATHERMAN_LOGGING_NAMESPACE);
    log("test", log_level::trace, 0, "testing %1% %2% %3%", 1, "2", 3.0);
    REQUIRE(context.level() == "TRACE");
    REQUIRE(context.message() == context.color() + "testing 1 2 3" + context.none());
    REQUIRE(context.ns() == "test");
    REQUIRE_FALSE(error_has_been_logged());
}

SCENARIO("logging with a DEBUG level") {
    logging_test_context context(log_level::debug);
    REQUIRE(LOG_IS_DEBUG_ENABLED());
    LOG_DEBUG("testing %1% %2% %3%", 1, "2", 3.0);
    REQUIRE(context.level() == "DEBUG");
    REQUIRE(context.message() == context.color() + "testing 1 2 3" + context.none());
    REQUIRE(context.ns() == LEATHERMAN_LOGGING_NAMESPACE);
    log("test", log_level::debug, 0, "testing %1% %2% %3%", 1, "2", 3.0);
    REQUIRE(context.level() == "DEBUG");
    REQUIRE(context.message() == context.color() + "testing 1 2 3" + context.none());
    REQUIRE(context.ns() == "test");
    REQUIRE_FALSE(error_has_been_logged());
}

SCENARIO("logging with an INFO level") {
    logging_test_context context(log_level::info);
    REQUIRE(LOG_IS_INFO_ENABLED());
    LOG_INFO("testing %1% %2% %3%", 1, "2", 3.0);
    REQUIRE(context.level() == "INFO");
    REQUIRE(context.message() == context.color() + "testing 1 2 3" + context.none());
    REQUIRE(context.ns() == LEATHERMAN_LOGGING_NAMESPACE);
    log("test", log_level::info, 0, "testing %1% %2% %3%", 1, "2", 3.0);
    REQUIRE(context.level() == "INFO");
    REQUIRE(context.message() == context.color() + "testing 1 2 3" + context.none());
    REQUIRE(context.ns() == "test");
    REQUIRE_FALSE(error_has_been_logged());
}

SCENARIO("logging with a WARNING level") {
    logging_test_context context(log_level::warning);
    REQUIRE(LOG_IS_WARNING_ENABLED());
    LOG_WARNING("testing %1% %2% %3%", 1, "2", 3.0);
    REQUIRE(context.level() == "WARN");
    REQUIRE(context.message() == context.color() + "testing 1 2 3" + context.none());
    REQUIRE(context.ns() == LEATHERMAN_LOGGING_NAMESPACE);
    log("test", log_level::warning, 0, "testing %1% %2% %3%", 1, "2", 3.0);
    REQUIRE(context.level() == "WARN");
    REQUIRE(context.message() == context.color() + "testing 1 2 3" + context.none());
    REQUIRE(context.ns() == "test");
    REQUIRE_FALSE(error_has_been_logged());
}

SCENARIO("logging with an ERROR level") {
    logging_test_context context(log_level::error);
    REQUIRE(LOG_IS_ERROR_ENABLED());
    LOG_ERROR("testing %1% %2% %3%", 1, "2", 3.0);
    REQUIRE(context.level() == "ERROR");
    REQUIRE(context.message() == context.color() + "testing 1 2 3" + context.none());
    REQUIRE(context.ns() == LEATHERMAN_LOGGING_NAMESPACE);
    log("test", log_level::error, 0, "testing %1% %2% %3%", 1, "2", 3.0);
    REQUIRE(context.level() == "ERROR");
    REQUIRE(context.message() == context.color() + "testing 1 2 3" + context.none());
    REQUIRE(context.ns() == "test");
    REQUIRE(error_has_been_logged());
}

SCENARIO("logging with a FATAL level") {
    logging_test_context context(log_level::fatal);
    REQUIRE(LOG_IS_FATAL_ENABLED());
    LOG_FATAL("testing %1% %2% %3%", 1, "2", 3.0);
    REQUIRE(context.level() == "FATAL");
    REQUIRE(context.message() == context.color() + "testing 1 2 3" + context.none());
    REQUIRE(context.ns() == LEATHERMAN_LOGGING_NAMESPACE);
    log("test", log_level::fatal, 0, "testing %1% %2% %3%", 1, "2", 3.0);
    REQUIRE(context.level() == "FATAL");
    REQUIRE(context.message() == context.color() + "testing 1 2 3" + context.none());
    REQUIRE(context.ns() == "test");
    REQUIRE(error_has_been_logged());
}

SCENARIO("logging with an override namespace") {
    logging_test_context context(log_level::warning, "custom");
    LOG_WARNING("testing");
    REQUIRE(context.ns() == "custom");
    log("test", log_level::warning, 0, "testing");
    REQUIRE(context.ns() == "test");
    REQUIRE_FALSE(error_has_been_logged());
}

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

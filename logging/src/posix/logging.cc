#include <leatherman/logging/logging.hpp>
#include <boost/nowide/iostream.hpp>
#include <unistd.h>
#include <syslog.h>

using namespace std;

namespace leatherman { namespace logging {

    void colorize(ostream& dst, log_level level)
    {
        if (!get_colorization()) {
            return;
        }

        static const string cyan = "\33[0;36m";
        static const string green = "\33[0;32m";
        static const string yellow = "\33[0;33m";
        static const string red = "\33[0;31m";
        static const string reset = "\33[0m";

        if (level == log_level::trace || level == log_level::debug) {
            dst << cyan;
        } else if (level == log_level::info) {
            dst << green;
        } else if (level == log_level::warning) {
            dst << yellow;
        } else if (level == log_level::error || level == log_level::fatal) {
            dst << red;
        } else {
            dst << reset;
        }
    }

    bool color_supported(ostream& dst)
    {
        return (&dst == &cout && isatty(fileno(stdout))) || (&dst == &cerr && isatty(fileno(stderr)));
    }

    void log_eventlog(log_level level, string const& message) {
        throw runtime_error("eventlog is available only on windows");
    }

    void setup_syslog_logging(string application, int facility)
    {
        openlog(application.c_str(), LOG_CONS | LOG_PID | LOG_NDELAY, facility);

        // Default to the warning level
        set_level(log_level::warning);
        enable_syslog();
    }

    int log_level_to_severity(log_level level)
    {
        switch (level) {
        case log_level::fatal:
            return LOG_ALERT;
        case log_level::error:
            return LOG_ERR;
        case log_level::warning:
            return LOG_WARNING;
        case log_level::info:
            return LOG_INFO;
        case log_level::debug:
        case log_level::trace:
        case log_level::none:
            return LOG_DEBUG;
        default:
            return LOG_INFO;
        }
    }

    void log_syslog(log_level level, string const &message) {
        int severity = log_level_to_severity(level);
        syslog(severity, "%s", message.c_str());
    }

    void clean_syslog_logging() { closelog(); }
}}  // namespace leatherman::logging

#include <leatherman/logging/logging.hpp>
#include <boost/nowide/iostream.hpp>
#include <unistd.h>
#include <syslog.h>

using namespace std;

namespace leatherman { namespace logging {

    namespace lth_locale = leatherman::locale;

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

    syslog_facility string_to_syslog_facility(std::string facility)
    {
        syslog_facility fac;
        try {
            const std::map<std::string, syslog_facility> option_to_syslog_facility {
                { "kern", syslog_facility::kern },
                { "user", syslog_facility::user },
                { "mail", syslog_facility::mail },
                { "daemon", syslog_facility::daemon },
                { "auth", syslog_facility::auth },
                { "syslog", syslog_facility::syslog },
                { "lpr", syslog_facility::lpr },
                { "news", syslog_facility::news },
                { "uucp", syslog_facility::uucp },
                { "cron", syslog_facility::cron },
                { "local0", syslog_facility::local0 },
                { "local1", syslog_facility::local1 },
                { "local2", syslog_facility::local2 },
                { "local3", syslog_facility::local3 },
                { "local4", syslog_facility::local4 },
                { "local5", syslog_facility::local5 },
                { "local6", syslog_facility::local6 },
                { "local7", syslog_facility::local7 },
            };
            fac = option_to_syslog_facility.at(facility);
        } catch (const std::out_of_range& e) {
            throw runtime_error {
                lth_locale::format("invalid syslog facility: '{1}'", facility) };
        }
        return fac;
    }

    void setup_syslog_logging(const char* application, const string& facility)
    {
        syslog_facility syslog_facility = string_to_syslog_facility(facility);
        int fac = static_cast<int>(syslog_facility);

        openlog(application, LOG_PID | LOG_NDELAY, fac);

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
            return LOG_DEBUG;
        default:
            return LOG_INFO;
        }
    }

    void log_syslog(log_level level, string const &message) {
        if (level != log_level::none) {
            int severity = log_level_to_severity(level);
            syslog(severity, "%s", message.c_str());
        }
    }

    void clean_syslog_logging() { closelog(); }
}}  // namespace leatherman::logging

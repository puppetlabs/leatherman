#include <leatherman/logging/logging.hpp>
#include <boost/nowide/iostream.hpp>
#include <windows.h>

#define STATUS_SEVERITY_SUCCESS          0x0
#define STATUS_SEVERITY_ERROR            0x1
#define STATUS_SEVERITY_WARNING          0x2
#define STATUS_SEVERITY_INFORMATIONAL    0x4

// Mark string for translation (alias for leatherman::locale::format)
using leatherman::locale::_;

using namespace std;

namespace leatherman { namespace logging {
    static HANDLE stdHandle;
    static WORD originalAttributes;

    static HANDLE h_event_log;

    void colorize(ostream& dst, log_level level)
    {
        if (!get_colorization()) {
            return;
        }

        // The ostream may have buffered data, and changing the console color will affect any buffered data written
        // later. Ensure the buffer is flushed before changing the console color.
        dst.flush();
        if (level == log_level::trace || level == log_level::debug) {
            SetConsoleTextAttribute(stdHandle, FOREGROUND_BLUE | FOREGROUND_GREEN);
        } else if (level == log_level::info) {
            SetConsoleTextAttribute(stdHandle, FOREGROUND_GREEN);
        } else if (level == log_level::warning) {
            SetConsoleTextAttribute(stdHandle, FOREGROUND_RED | FOREGROUND_GREEN);
        } else if (level == log_level::error || level == log_level::fatal) {
            SetConsoleTextAttribute(stdHandle, FOREGROUND_RED);
        } else {
            SetConsoleTextAttribute(stdHandle, originalAttributes);
        }
    }

    bool color_supported(ostream& dst)
    {
        bool colorize = false;
        if (&dst == &cout || &dst == &boost::nowide::cout) {
            stdHandle = GetStdHandle(STD_OUTPUT_HANDLE);
            colorize = true;
        } else if (&dst == &cerr || &dst == &boost::nowide::cerr) {
            stdHandle = GetStdHandle(STD_ERROR_HANDLE);
            colorize = true;
        }

        if (colorize) {
            CONSOLE_SCREEN_BUFFER_INFO csbiInfo;
            GetConsoleScreenBufferInfo(stdHandle, &csbiInfo);
            originalAttributes = csbiInfo.wAttributes;
        }
        return colorize;
    }

    void setup_eventlog_logging(string application)
    {
        const wstring w_application(application.begin(), application.end());

        // create registry keys for ACLing described on MSDN:
        // http://msdn2.microsoft.com/en-us/library/aa363648.aspx
        h_event_log = RegisterEventSource(NULL, w_application.c_str());
        if (NULL == h_event_log) {
            throw runtime_error(
                _("RegisterEventSource failed with 0x%x.\n", GetLastError()));
        }
        // Default to the warning level
        set_level(log_level::warning);
        enable_event_log();
    }

    int log_level_to_severity(log_level level)
    {
        switch (level) {
        case log_level::fatal:
        case log_level::error:
            return STATUS_SEVERITY_ERROR;
        case log_level::warning:
            return STATUS_SEVERITY_WARNING;
        case log_level::info:
        case log_level::debug:
        case log_level::trace:
            return STATUS_SEVERITY_INFORMATIONAL;
        case log_level::none:
            return STATUS_SEVERITY_SUCCESS;
        }
        return STATUS_SEVERITY_SUCCESS;
    }

    void log_eventlog(log_level level, string const &message)
    {
        if (h_event_log) {
            int severity = log_level_to_severity(level);
            if (severity != STATUS_SEVERITY_SUCCESS) {
                const int category = 0;
                const int event_id = 1;
                const wstring w_message(message.begin(), message.end());
                LPCWSTR p_w_message = w_message.c_str();
                if (!ReportEvent(h_event_log, severity, category, event_id, NULL, 1,
                                 0, &p_w_message, NULL)) {
                    // this message won't show up when running as a service
                    boost::nowide::cerr << "ReportEvent failed with " << GetLastError()
                                        << ", msg: " << message << "\n";
                }
            }
        }
    }

    void clean_eventlog_logging()
    {
        if (h_event_log) {
            DeregisterEventSource(h_event_log);
            h_event_log = NULL;
            disable_event_log();
        }
    }
}}  // namespace leatherman::logging

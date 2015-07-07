#include <leatherman/logging/logging.hpp>
#include <leatherman/locale/locale.hpp>
#include <vector>

// boost includes are not always warning-clean. Disable warnings that
// cause problems before including the headers, then re-enable the warnings.
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wextra"

#include <boost/log/utility/setup/common_attributes.hpp>
#include <boost/log/attributes/scoped_attribute.hpp>
#include <boost/log/sources/severity_logger.hpp>
#include <boost/log/sources/record_ostream.hpp>
#include <boost/log/sinks/sync_frontend.hpp>
#include <boost/log/sinks/basic_sink_backend.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/date_time/gregorian/gregorian.hpp>

#pragma GCC diagnostic pop

using namespace std;
namespace expr = boost::log::expressions;
namespace src = boost::log::sources;
namespace attrs = boost::log::attributes;
namespace keywords = boost::log::keywords;
namespace sinks = boost::log::sinks;

namespace leatherman { namespace logging {

    static function<bool(LogLevel, string const&)> g_callback;
    static LogLevel g_level = LogLevel::none;
    static bool g_colorize = false;
    static bool g_error_logged = false;

    class color_writer : public sinks::basic_sink_backend<sinks::synchronized_feeding>
    {
     public:
        color_writer(ostream *dst);
        void consume(boost::log::record_view const& rec);
     private:
        ostream &_dst;
    };

    color_writer::color_writer(ostream *dst) : _dst(*dst) {}

    void color_writer::consume(boost::log::record_view const& rec)
    {
        auto level = boost::log::extract<LogLevel>("Severity", rec);
        auto line_num = boost::log::extract<int>("LineNum", rec);
        auto name_space = boost::log::extract<string>("Namespace", rec);
        auto timestamp = boost::log::extract<boost::posix_time::ptime>("TimeStamp", rec);
        auto message = rec[expr::smessage];

        _dst << boost::gregorian::to_iso_extended_string(timestamp->date());
        _dst << " " << boost::posix_time::to_simple_string(timestamp->time_of_day());
        _dst << " " << left << setfill(' ') << setw(5) << level << " " << *name_space;
        if (line_num) {
            _dst << ":" << *line_num;
        }
        _dst << " - ";
        colorize(_dst, *level);
        _dst << *message;
        colorize(_dst);
        _dst << endl;
    }

    void setup_logging(ostream &dst, string locale)
    {
        // Remove existing sinks before adding a new one
        auto core = boost::log::core::get();
        core->remove_all_sinks();

        using sink_t = sinks::synchronous_sink<color_writer>;
        boost::shared_ptr<sink_t> sink(new sink_t(&dst));
        core->add_sink(sink);

#if !defined(__sun) || !defined(__GNUC__)
        // Imbue the logging sink with the requested locale.
        // Locale in GCC is busted on Solaris, so skip it.
        dst.imbue(leatherman::locale::get_locale(locale));
#endif

        boost::log::add_common_attributes();

        // Default to the warning level
        set_level(LogLevel::warning);

        // Set whether or not to use colorization depending if the destination is a tty
        g_colorize = color_supported(dst);
    }

    void set_level(LogLevel level)
    {
        auto core = boost::log::core::get();
        core->set_logging_enabled(level != LogLevel::none);
        g_level = level;
    }

    LogLevel get_level()
    {
        return g_level;
    }

    void set_colorization(bool color)
    {
        g_colorize = color;
    }

    bool get_colorization()
    {
        return g_colorize;
    }

    bool is_enabled(LogLevel level)
    {
        return g_level != LogLevel::none && static_cast<int>(level) >= static_cast<int>(g_level);
    }

    bool error_has_been_logged() {
        return g_error_logged;
    }

    void clear_error_logged_flag() {
        g_error_logged = false;
    }

    void on_message(function<bool(LogLevel, string const&)> callback)
    {
        g_callback = callback;
    }

    void log(const string &logger, LogLevel level, int line_num, boost::format& message)
    {
        log(logger, level, line_num, message.str());
    }

    void log(const string &logger, LogLevel level, int line_num, string const& message)
    {
        if (level >= LogLevel::error) {
            g_error_logged = true;
        }
        if (!is_enabled(level) || (g_callback && !g_callback(level, message))) {
            return;
        }

        src::severity_logger<LogLevel> slg;
        slg.add_attribute("Namespace", attrs::constant<string>(logger));
        if (line_num > 0) {
            slg.add_attribute("LineNum", attrs::constant<int>(line_num));
        }

        BOOST_LOG_SEV(slg, level) << message;
    }

    istream& operator>>(istream& in, LogLevel & level)
    {
        string value;
        if (in >> value) {
            if (value == "none") {
                level = LogLevel::none;
                return in;
            }
            if (value == "trace") {
                level = LogLevel::trace;
                return in;
            }
            if (value == "debug") {
                level = LogLevel::debug;
                return in;
            }
            if (value == "info") {
                level = LogLevel::info;
                return in;
            }
            if (value == "warn") {
                level = LogLevel::warning;
                return in;
            }
            if (value == "error") {
                level = LogLevel::error;
                return in;
            }
            if (value == "fatal") {
                level = LogLevel::fatal;
                return in;
            }
        }
        throw runtime_error("invalid log level: expected none, trace, debug, info, warn, error, or fatal.");
    }

    ostream& operator<<(ostream& strm, LogLevel level)
    {
        static const vector<string> strings = {"TRACE", "DEBUG", "INFO", "WARN", "ERROR", "FATAL"};

        if (level != LogLevel::none) {
            size_t index = static_cast<size_t>(level) - 1;
            if (index < strings.size()) {
                strm << strings[index];
            }
        }

        return strm;
    }

}}  // namespace leatherman::logging

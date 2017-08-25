#include <leatherman/logging/logging.hpp>
#include <leatherman/locale/locale.hpp>
#include <vector>

// Mark string for translation (alias for leatherman::locale::format)
using leatherman::locale::_;

// boost includes are not always warning-clean. Disable warnings that
// cause problems before including the headers, then re-enable the warnings.
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wextra"

#include <boost/log/utility/setup/common_attributes.hpp>
#include <boost/log/attributes/scoped_attribute.hpp>
#include <boost/log/sources/logger.hpp>
#include <boost/log/sources/record_ostream.hpp>
#include <boost/log/sinks/sync_frontend.hpp>
#include <boost/log/sinks/basic_sink_backend.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/date_time/gregorian/gregorian.hpp>
#include <boost/algorithm/string.hpp>

#pragma GCC diagnostic pop

using namespace std;
namespace expr = boost::log::expressions;
namespace src = boost::log::sources;
namespace attrs = boost::log::attributes;
namespace keywords = boost::log::keywords;
namespace sinks = boost::log::sinks;

namespace leatherman { namespace logging {

    static function<bool(log_level, string const&)> g_callback;
    static log_level g_level = log_level::none;
    static bool g_colorize = false;
    static bool g_error_logged = false;

    namespace lth_locale = leatherman::locale;

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
        auto level = boost::log::extract<log_level>("Severity", rec);

        if (!is_enabled(*level)) {
            return;
        }

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

    // cppcheck-suppress passedByValue
    void setup_logging(ostream &dst, string locale, string domain, bool use_locale)
    {
        // Remove existing sinks before adding a new one
        auto core = boost::log::core::get();
        core->remove_all_sinks();

        using sink_t = sinks::synchronous_sink<color_writer>;
        boost::shared_ptr<sink_t> sink = boost::make_shared<sink_t>(boost::make_shared<color_writer>(&dst));
        core->add_sink(sink);


#ifdef LEATHERMAN_USE_LOCALES
        // Imbue the logging sink with the requested locale.
        // Locale in GCC is busted on Solaris, so skip it.
        // TODO: Imbue may not be useful, as setup_logging can be called multiple times
        // with different domains for the same ostream.
        // Note that this creates a locale that's not usable for testing, as it
        // only includes paths for install locations. This is intentional, to avoid leaving
        // searching paths that have unknown permissions.
        if (use_locale) {
            dst.imbue(lth_locale::get_locale(locale, domain, {}));
        }
#endif

        boost::log::add_common_attributes();

        // Default to the warning level
        set_level(log_level::warning);

        // Set whether or not to use colorization depending if the destination is a tty
        g_colorize = color_supported(dst);
    }

    // This version exists for binary compatibility only.
    void setup_logging(ostream &dst, string locale, string domain)
    {
        setup_logging(dst, move(locale), move(domain), true);
    }

    void set_level(log_level level)
    {
        auto core = boost::log::core::get();
        core->set_logging_enabled(level != log_level::none);
        g_level = level;
    }

    log_level get_level()
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

    bool is_enabled(log_level level)
    {
        return g_level != log_level::none && static_cast<int>(level) >= static_cast<int>(g_level);
    }

    bool error_has_been_logged() {
        return g_error_logged;
    }

    void clear_error_logged_flag() {
        g_error_logged = false;
    }

    // cppcheck-suppress passedByValue
    void on_message(function<bool(log_level, string const&)> callback)
    {
        g_callback = callback;
    }

    void log_helper(const string &logger, log_level level, int line_num, string const& message)
    {
        if (level >= log_level::error) {
            g_error_logged = true;
        }
        if (!is_enabled(level) || (g_callback && !g_callback(level, message))) {
            return;
        }

        src::logger slg;
        slg.add_attribute("Severity", attrs::constant<log_level>(level));
        slg.add_attribute("Namespace", attrs::constant<string>(logger));
        if (line_num > 0) {
            slg.add_attribute("LineNum", attrs::constant<int>(line_num));
        }

        BOOST_LOG(slg) << message;
    }

    istream& operator>>(istream& in, log_level& level)
    {
        string value;
        if (in >> value) {
            boost::algorithm::to_lower(value);
            if (value == "none") {
                level = log_level::none;
                return in;
            }
            if (value == "trace") {
                level = log_level::trace;
                return in;
            }
            if (value == "debug") {
                level = log_level::debug;
                return in;
            }
            if (value == "info") {
                level = log_level::info;
                return in;
            }
            if (value == "warn") {
                level = log_level::warning;
                return in;
            }
            if (value == "error") {
                level = log_level::error;
                return in;
            }
            if (value == "fatal") {
                level = log_level::fatal;
                return in;
            }
        }
        throw runtime_error(_("invalid log level '{1}': expected none, trace, debug, info, warn, error, or fatal.", value));
    }

    ostream& operator<<(ostream& strm, log_level level)
    {
        static const vector<string> strings = {"TRACE", "DEBUG", "INFO", "WARN", "ERROR", "FATAL"};

        if (level != log_level::none) {
            size_t index = static_cast<size_t>(level) - 1;
            if (index < strings.size()) {
                strm << strings[index];
            }
        }

        return strm;
    }

}}  // namespace leatherman::logging

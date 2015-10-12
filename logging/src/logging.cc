#include <leatherman/logging/logging.hpp>
#include <leatherman/locale/locale.hpp>
#include <vector>
#include <locale>

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
#include <boost/log/sinks/text_ostream_backend.hpp>
#include <boost/log/sinks/text_file_backend.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/date_time/gregorian/gregorian.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/filesystem/path.hpp>

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

    template <typename StreamT>
    static void format_record(boost::log::record_view const& rec, StreamT& strm)
    {
        auto level = boost::log::extract<log_level>("Severity", rec);
        auto line_num = boost::log::extract<int>("LineNum", rec);
        auto name_space = boost::log::extract<string>("Namespace", rec);
        auto timestamp = boost::log::extract<boost::posix_time::ptime>("TimeStamp", rec);
        auto message = rec[expr::smessage];

        strm << boost::gregorian::to_iso_extended_string(timestamp->date());
        strm << " " << boost::posix_time::to_simple_string(timestamp->time_of_day());
        strm << " " << left << setfill(' ') << setw(5) << level << " " << *name_space;
        if (line_num) {
            strm << ":" << *line_num;
        }
        strm << " - ";
        colorize(strm, *level);
        strm << *message;
        colorize(strm);
    }

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
        format_record<ostream>(rec, _dst);
        _dst << endl;
    }

    void color_formatter(boost::log::record_view const& rec,
                         boost::log::formatting_ostream& strm)
    {
        format_record<boost::log::formatting_ostream>(rec, strm);
    }

    template <typename SinkT>
    static void setup_common_logging_config(boost::shared_ptr<SinkT> sink)
    {
        // Remove existing sinks before adding a new one
        auto core = boost::log::core::get();
        core->remove_all_sinks();

        core->add_sink(sink);

        boost::log::add_common_attributes();

        // Default to the warning level
        set_level(log_level::warning);
    }

    void setup_logging(ostream &dst, string locale)
    {
        using sink_t = sinks::synchronous_sink<color_writer>;
        boost::shared_ptr<sink_t> sink(new sink_t(&dst));

#if (!defined(__sun) && !defined(_AIX)) || !defined(__GNUC__)
        // Imbue the logging sink with the requested locale.
        // Locale in GCC is busted on Solaris, so skip it.
        dst.imbue(leatherman::locale::get_locale(locale));
#endif

        setup_common_logging_config<sink_t>(sink);

        // Set whether or not to use colorization depending if the destination is a tty
        g_colorize = color_supported(dst);
    }

    void setup_logging(std::string file_name,
                       std::ios_base::openmode open_mode,
                       std::string target,
                       int rotation_size,
                       int max_size,
                       int min_free_space,
                       std::string locale)
    {
        // Initialize locale to default before boost::log in order to avoid
        // crashes during process termination due to file sinks; refer to
        // www.boost.org/doc/libs/1_59_0/libs/log/doc/html/log/rationale/why_crash_on_term.html
        boost::filesystem::path::imbue(std::locale("C"));

        // Use the text file as logging backend, in order to set file rotation
        using backend_t = sinks::text_file_backend;
        boost::shared_ptr<backend_t> backend(
            new backend_t(
                keywords::file_name = file_name.data(),     // file name pattern
                keywords::open_mode = open_mode,            // file open mode
                keywords::rotation_size = rotation_size,    // triggers rotation, [char]
                keywords::auto_flush = true));              // flushes at every record

        using sink_t = sinks::synchronous_sink<backend_t>;
        boost::shared_ptr<sink_t> sink(new sink_t(backend));

        // Configure the file collector; this will limit the size of log files
        sink->locked_backend()->set_file_collector(sinks::file::make_collector(
            keywords::target = target.data(),               // dir for rotated files
            keywords::max_size = max_size,                  // max for stored files, [byte]
            keywords::min_free_space = min_free_space));    // min drive space, [byte]

        // Scan old files that match the name pattern; consider them already rotated
        sink->locked_backend()->scan_for_files();

        sink->set_formatter(&color_formatter);

#if (!defined(__sun) && !defined(_AIX)) || !defined(__GNUC__)
        // Imbue the logging sink with the requested locale.
        // Locale in GCC is busted on Solaris, so skip it.
        sink->imbue(leatherman::locale::get_locale(locale));
#endif

        setup_common_logging_config<sink_t>(sink);

        // By default, don't use colorization when writing on file; on POSIX,
        // the user can still enable it by calling set_colorization(true)
        g_colorize = false;
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

    void on_message(function<bool(log_level, string const&)> callback)
    {
        g_callback = callback;
    }

    void log(const string &logger, log_level level, int line_num, boost::format& message)
    {
        log(logger, level, line_num, message.str());
    }

    void log(const string &logger, log_level level, int line_num, string const& message)
    {
        if (level >= log_level::error) {
            g_error_logged = true;
        }
        if (!is_enabled(level) || (g_callback && !g_callback(level, message))) {
            return;
        }

        src::severity_logger<log_level> slg;
        slg.add_attribute("Namespace", attrs::constant<string>(logger));
        if (line_num > 0) {
            slg.add_attribute("LineNum", attrs::constant<int>(line_num));
        }

        BOOST_LOG_SEV(slg, level) << message;
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
        throw runtime_error((boost::format("invalid log level '%1%': expected none, trace, debug, info, warn, error, or fatal.") % value).str());
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

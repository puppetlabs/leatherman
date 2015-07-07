#include "logging.hpp"
#include <boost/algorithm/string/predicate.hpp>
#include <boost/nowide/iostream.hpp>

namespace boost {

    bool operator== (boost::regex const& lhs, std::string const& rhs)
    {
        return boost::regex_match(rhs, lhs);
    }

    bool operator== (std::string const& lhs, boost::regex const& rhs)
    {
        return boost::regex_match(lhs, rhs);
    }

}  // namespace boost

namespace leatherman { namespace test {

    static bool all_spaces(string const& s)
    {
        return boost::algorithm::all(s, [](char c) { return c == ' '; });
    }

    std::streamsize colored_tokenizing_stringbuf::xsputn(char_type const* s, std::streamsize count)
    {
        auto str = string(s, count);
        if (all_spaces(str) && !tokens.empty() && all_spaces(tokens.back())) {
            // Lump all white space strings together.
            tokens.back() += move(str);
        } else {
            tokens.emplace_back(move(str));
        }

        return stringbuf::xsputn(s, count);
    }

    logging_format_context::logging_format_context(LogLevel lvl, string ns, int line_num)
    {
        _strm_buf = boost::nowide::cout.rdbuf();
        boost::nowide::cout.rdbuf(&_buf);

        setup_logging(boost::nowide::cout);
        set_level(LogLevel::trace);
        set_colorization(true);
        clear_error_logged_flag();

        stringstream lvl_str;
        lvl_str << lvl;

        using R = boost::regex;
        static const boost::regex rdate("\\d{4}-\\d{2}-\\d{2}");
        static const boost::regex rtime("[0-2]\\d:[0-5]\\d:\\d{2}\\.\\d{6}");

        _expected = {rdate, R(" "), rtime, R(" "), R(lvl_str.str()), R("[ ]+"), R(ns, R::literal)};

        if (line_num > 0) {
            _expected.emplace_back(":");
            _expected.emplace_back(to_string(line_num));
        }
        _expected.emplace_back(" - ");

        auto color = get_color(lvl);
        if (!color.empty()) {
            _expected.emplace_back(color, R::literal);
        }
        _expected.emplace_back("testing 1 2 3");
        if (!color.empty()) {
            _expected.emplace_back(get_color(LogLevel::none), R::literal);
        }
    }

    logging_format_context::~logging_format_context()
    {
        boost::nowide::cout.rdbuf(_strm_buf);

        set_level(LogLevel::none);
        on_message(nullptr);
        clear_error_logged_flag();

        auto core = boost::log::core::get();
        core->reset_filter();
        core->remove_all_sinks();
    }

    vector<string> const& logging_format_context::tokens() const
    {
        return _buf.tokens;
    }

    string logging_format_context::message() const
    {
        return _buf.str();
    }

    vector<boost::regex> const& logging_format_context::expected() const
    {
        return _expected;
    }

}}  // namespace leatherman::test

#include "logging.hpp"
#include <boost/algorithm/string/predicate.hpp>
#include <boost/nowide/iostream.hpp>
#include <cassert>

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

    logging_context::logging_context(log_level lvl)
    {
        set_level(lvl);
        REQUIRE(get_level() == lvl);
        clear_error_logged_flag();
    }

    logging_context::~logging_context()
    {
        set_level(log_level::none);
        REQUIRE(get_level() == log_level::none);
        on_message(nullptr);
        clear_error_logged_flag();
    }

    logging_format_context::logging_format_context(log_level lvl, string ns, int line_num)
        : logging_context(lvl)
    {
        _strm_buf = boost::nowide::cout.rdbuf();
        boost::nowide::cout.rdbuf(&_buf);

        setup_logging(boost::nowide::cout);
        set_level(lvl);
        REQUIRE(get_level() == lvl);
        set_colorization(true);

        static const boost::regex rdate("\\d{4}-\\d{2}-\\d{2}");
        static const boost::regex rtime("[0-2]\\d:[0-5]\\d:\\d{2}\\.\\d{6}");

        _expected = {rdate, " ", rtime, " ", lvl, boost::regex("[ ]+"), ns};

        if (line_num > 0) {
            _expected.emplace_back(":");
            _expected.emplace_back(to_string(line_num));
        }
        _expected.emplace_back(" - ");

        auto color = get_color(lvl);
        if (!color.empty()) {
            _expected.emplace_back(color);
        }
        _expected.emplace_back("testing 1 2 3");
        if (!color.empty()) {
            _expected.emplace_back(get_color(log_level::none));
        }
    }

    logging_format_context::~logging_format_context()
    {
        boost::nowide::cout.rdbuf(_strm_buf);

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

    vector<matcher> const& logging_format_context::expected() const
    {
        return _expected;
    }

}}  // namespace leatherman::test

namespace boost {

    bool operator== (std::string const& lhs, leatherman::test::matcher const& rhs)
    {
        using leatherman::logging::log_level;
        if (auto *expected = boost::get<boost::regex>(&rhs)) {
            return boost::regex_match(lhs, *expected);
        } else if (auto *expected = boost::get<log_level>(&rhs)) {
            std::stringstream ss{lhs};
            log_level lvl = log_level::none;
            ss >> lvl;
            return lvl == *expected;
        } else if (auto *expected = boost::get<std::string>(&rhs)) {
            return lhs == *expected;
        } else {
            return false;
        }
    }

}  // namespace boost

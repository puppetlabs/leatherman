/**
 * @file Utilities for testing logging
 */
#pragma once
#include <catch.hpp>
#include <leatherman/logging/logging.hpp>
#include <boost/regex.hpp>
#include <boost/iterator/zip_iterator.hpp>
#include <boost/range.hpp>
#include <boost/variant.hpp>

namespace leatherman { namespace test {
    using namespace std;
    using namespace leatherman::logging;

    using matcher = boost::variant<boost::regex, string, log_level>;

    /**
     * Declare colors.
     */
    constexpr char cyan[] = "\33[0;36m";
    constexpr char green[] = "\33[0;32m";
    constexpr char yellow[] = "\33[0;33m";
    constexpr char red[] = "\33[0;31m";
    constexpr char reset[] = "\33[0m";

    /**
     * Zip view for iterating over multiple containers at once
     */
    template <typename... T>
    auto zip_view(T const&... containers) ->
        boost::iterator_range<boost::zip_iterator<decltype(boost::make_tuple(std::begin(containers)...))>>
    {
        auto zip_begin = boost::make_zip_iterator(boost::make_tuple(std::begin(containers)...));
        auto zip_end = boost::make_zip_iterator(boost::make_tuple(std::end(containers)...));
        return boost::make_iterator_range(zip_begin, zip_end);
    }

    /**
     * Stringbuf for capturing tokens written to the attached stream, and insert color
     * codes on Windows that match to the current console attributes.
     */
    class colored_tokenizing_stringbuf : public stringbuf
    {
     public:
        vector<string> tokens;

     protected:
        virtual std::streamsize xsputn(char_type const* s, std::streamsize count);
    };

    /**
     * Context for simple logging tests.
     */
    struct logging_context
    {
        logging_context(log_level lvl = log_level::trace);
        virtual ~logging_context();
    };

    /**
     * Context for capturing the format of log messages as they would appear on cout/cerr.
     */
    struct logging_format_context : logging_context
    {
        logging_format_context(log_level lvl = log_level::trace, string ns = LOG_NAMESPACE, int line_num = 0);
        ~logging_format_context() final;

        vector<string> const& tokens() const;
        string message() const;

        vector<matcher> const& expected() const;

     private:
        string get_color(log_level lvl) const;

        colored_tokenizing_stringbuf _buf;
        streambuf *_strm_buf;
        vector<matcher> _expected;
    };

}}  // namespace leatherman::test

namespace boost {

    bool operator== (std::string const& lhs, leatherman::test::matcher const& rhs);

}  // namespace boost

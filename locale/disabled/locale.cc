#include <leatherman/locale/locale.hpp>

namespace leatherman { namespace locale {

    using namespace std;

    const std::locale get_locale(string const& id, string const& domain, vector<string> const& paths)
    {
        // std::locale is not supported on these platforms
        throw runtime_error("leatherman::locale::get_locale is not supported on this platform");
    }

    void clear_domain(string const& domain)
    {
        throw runtime_error("leatherman::locale::clear_domain is not supported on this platform");
    }

    string translate(string const& msg, string const& domain)
    {
        return msg;
    }

    string translate_p(string const& context, string const& msg, string const& domain)
    {
        return msg;
    }

    string translate_n(string const& single, string const& plural, int n, string const& domain)
    {
        return n == 1 ? single : plural;
    }

    string translate_np(string const& context, string const& single, string const& plural, int n, string const& domain)
    {
        return n == 1 ? single : plural;
    }

}}  // namespace leatherman::locale

#include <leatherman/logging/logging.hpp>
#include <boost/nowide/iostream.hpp>
#include <unistd.h>

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

}}  // namespace leatherman::logging

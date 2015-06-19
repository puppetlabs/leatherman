#include "../logging.hpp"

namespace leatherman { namespace test {

    string logging_format_context::get_color(log_level lvl) const
    {
        switch (lvl) {
            case log_level::trace:
            case log_level::debug:
                return cyan;
            case log_level::info:
                return green;
            case log_level::warning:
                return yellow;
            case log_level::error:
            case log_level::fatal:
                return red;
            default:
                return reset;
        }
    }

}}  // namespace leatherman::test

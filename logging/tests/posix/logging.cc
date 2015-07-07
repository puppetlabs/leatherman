#include "../logging.hpp"

namespace leatherman { namespace test {

    string logging_format_context::get_color(LogLevel lvl) const
    {
        switch (lvl) {
            case LogLevel::trace:
            case LogLevel::debug:
                return cyan;
            case LogLevel::info:
                return green;
            case LogLevel::warning:
                return yellow;
            case LogLevel::error:
            case LogLevel::fatal:
                return red;
            default:
                return reset;
        }
    }

}}  // namespace leatherman::test

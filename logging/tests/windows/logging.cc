#include "../logging.hpp"
#include <windows.h>

namespace leatherman { namespace test {

    string logging_format_context::get_color(log_level lvl) const
    {
        // So far I've found no way to successfully test console color when output is redirected,
        // as happens when running with ctest or make test.
        return "";
    }

}}  // namespace leatherman::test

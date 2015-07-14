#include "log_capture.hpp"
#include <boost/nowide/iostream.hpp>

using namespace std;
using namespace leatherman::logging;

namespace leatherman { namespace execution { namespace testing {

    log_capture::log_capture(log_level level)
    {
        // Setup logging for capturing
        setup_logging(_stream);
        set_level(level);
    }

    log_capture::~log_capture()
    {
        // Cleanup
        setup_logging(boost::nowide::cout);
        set_level(log_level::none);
        clear_error_logged_flag();
    }

    string log_capture::result() const
    {
        return _stream.str();
    }

}}}  // namespace leatherman::execution::testing

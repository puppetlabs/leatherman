#include <leatherman/logging/logging.hpp>
#include <boost/nowide/iostream.hpp>
#include <windows.h>

using namespace std;

namespace leatherman { namespace logging {
    static HANDLE stdHandle;
    static WORD originalAttributes;

    void colorize(ostream& dst, LogLevel level)
    {
        if (!get_colorization()) {
            return;
        }

        // The ostream may have buffered data, and changing the console color will affect any buffered data written
        // later. Ensure the buffer is flushed before changing the console color.
        dst.flush();
        if (level == LogLevel::trace || level == LogLevel::debug) {
            SetConsoleTextAttribute(stdHandle, FOREGROUND_BLUE | FOREGROUND_GREEN);
        } else if (level == LogLevel::info) {
            SetConsoleTextAttribute(stdHandle, FOREGROUND_GREEN);
        } else if (level == LogLevel::warning) {
            SetConsoleTextAttribute(stdHandle, FOREGROUND_RED | FOREGROUND_GREEN);
        } else if (level == LogLevel::error || level == LogLevel::fatal) {
            SetConsoleTextAttribute(stdHandle, FOREGROUND_RED);
        } else {
            SetConsoleTextAttribute(stdHandle, originalAttributes);
        }
    }

    bool color_supported(ostream& dst)
    {
        bool colorize = false;
        if (&dst == &cout || &dst == &boost::nowide::cout) {
            stdHandle = GetStdHandle(STD_OUTPUT_HANDLE);
            colorize = true;
        } else if (&dst == &cerr || &dst == &boost::nowide::cerr) {
            stdHandle = GetStdHandle(STD_ERROR_HANDLE);
            colorize = true;
        }

        if (colorize) {
            CONSOLE_SCREEN_BUFFER_INFO csbiInfo;
            GetConsoleScreenBufferInfo(stdHandle, &csbiInfo);
            originalAttributes = csbiInfo.wAttributes;
        }
        return colorize;
    }

}}  // namespace leatherman::logging

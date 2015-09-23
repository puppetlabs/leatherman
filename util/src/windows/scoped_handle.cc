#include <leatherman/util/windows/scoped_handle.hpp>
#include <windows.h>

using namespace std;

namespace leatherman { namespace util { namespace windows {

    scoped_handle::scoped_handle(HANDLE h) :
        scoped_resource(h, close)
    {
    }

    scoped_handle::scoped_handle() : scoped_resource(INVALID_HANDLE_VALUE, nullptr)
    {
    }

    void scoped_handle::close(HANDLE handle)
    {
        if (handle != INVALID_HANDLE_VALUE) {
            CloseHandle(handle);
        }
    }

}}}  // namespace leatherman::util::windows

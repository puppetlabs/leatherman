#include <leatherman/util/scoped_resource.hpp>
#include <leatherman/windows/system_error.hpp>
#include <leatherman/windows/windows.hpp>
#include <leatherman/locale/locale.hpp>
#include <boost/nowide/convert.hpp>

// Mark string for translation (alias for leatherman::locale::format)
using leatherman::locale::_;

using namespace std;

namespace leatherman { namespace windows {

    string system_error(DWORD err)
    {
        LPWSTR buffer = nullptr;
        if (FormatMessageW(
            FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_IGNORE_INSERTS,
            nullptr, err, 0, reinterpret_cast<LPWSTR>(&buffer), 0, nullptr) == 0 || !buffer) {
            return _("unknown error ({1})", err);
        }

        // boost format could throw, so ensure the buffer is freed.
        util::scoped_resource<LPWSTR> guard(buffer, [](LPWSTR ptr) { if (ptr) LocalFree(ptr); });
        return _("{1} ({2})", boost::nowide::narrow(buffer), err);
    }

    string system_error()
    {
        return system_error(GetLastError());
    }

}}  // namespace leatherman::windows

#include <leatherman/windows/file_util.hpp>
#include <leatherman/windows/system_error.hpp>
#include <leatherman/windows/windows.hpp>
#include <leatherman/locale/locale.hpp>
#include <boost/filesystem.hpp>

#include <shlobj.h>

using namespace std;
using namespace boost::filesystem;
namespace lth_locale = leatherman::locale;

namespace leatherman { namespace windows { namespace file_util {

    string get_programdata_dir() {
        PWSTR pdir;
	if (SUCCEEDED(SHGetKnownFolderPath(FOLDERID_ProgramData, 0, nullptr, &pdir))) {
	    auto p = path(pdir);
	    return p.string();
	}
	throw unknown_folder_exception(lth_locale::format("error finding FOLDERID_ProgramData: {1}", 
            leatherman::windows::system_error()));
    }

}}}  // namespace leatherman::windows::file_util


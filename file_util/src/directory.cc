#include <leatherman/file_util/directory.hpp>
#include <leatherman/util/regex.hpp>
#include <boost/filesystem.hpp>

using namespace std;
using namespace boost::filesystem;
using namespace leatherman::util;

namespace leatherman { namespace file_util {

    static void each(string const& directory, file_type type, function<bool(string const&)> const& callback, string const& pattern)
    {
        boost::regex regex;
        if (!pattern.empty()) {
            regex = pattern;
        }

        // Attempt to iterate the directory
        boost::system::error_code ec;
        directory_iterator it = directory_iterator(directory, ec);
        if (ec) {
            return;
        }

        // Call the callback for any matching entries
        directory_iterator end;
        for (; it != end; ++it) {
            ec.clear();
            auto status = it->status(ec);
            if (ec || (status.type() != type)) {
                continue;
            }
            if (regex.empty() || re_search(it->path().filename().string(), regex)) {
                if (!callback(it->path().string())) {
                    break;
                }
            }
        }
    }

    void each_file(string const& directory, function<bool(string const&)> const& callback, string const& pattern)
    {
        each(directory, regular_file, callback, pattern);
    }

    void each_subdirectory(string const& directory, function<bool(string const&)> const& callback, string const& pattern)
    {
        each(directory, directory_file, callback, pattern);
    }

}}  // namespace leatherman::file_util

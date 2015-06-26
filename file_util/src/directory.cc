#include <leatherman/file_util/directory.hpp>
#include <leatherman/util/regex.hpp>
#include <boost/filesystem.hpp>

using namespace std;
using namespace boost::filesystem;

namespace leatherman { namespace file_util {

    void each_file(string const& directory, function<bool(string const&)> callback, string const& pattern)
    {
        boost::regex regex(pattern);

        // Attempt to iterate the directory
        boost::system::error_code ec;
        directory_iterator it = directory_iterator(directory, ec);
        if (ec) {
            return;
        }

        // Call the callback for any matching files
        directory_iterator end;
        for (; it != end; ++it) {
            boost::system::error_code ec;
            if (!is_regular_file(it->status(ec))) {
                continue;
            }
            if (re_search(it->path().filename().string(), regex)) {
                if (!callback(it->path().string())) {
                    break;
                }
            }
        }
    }

    void each_subdirectory(string const& directory, function<bool(string const&)> callback, string const& pattern)
    {
        boost::regex regex(pattern);

        // Attempt to iterate the directory
        boost::system::error_code ec;
        directory_iterator it = directory_iterator(directory, ec);
        if (ec) {
            return;
        }

        // Call the callback for any matching subdirectories
        directory_iterator end;
        for (; it != end; ++it) {
            boost::system::error_code ec;
            if (!is_directory(it->status(ec))) {
                continue;
            }
            if (re_search(it->path().filename().string(), regex)) {
                if (!callback(it->path().string())) {
                    break;
                }
            }
        }
    }

}}  // namespace leatherman::file_util

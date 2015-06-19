#include <leatherman/file_util/directory.hpp>
#include <leatherman/file_util/regex.hpp>
#include <boost/filesystem.hpp>

using namespace std;
using namespace boost::filesystem;

namespace leatherman { namespace file_util {

    void directory::each_file(string const& directory, function<bool(string const&)> callback, string const& pattern)
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

    void directory::each_subdirectory(string const& directory, function<bool(string const&)> callback, string const& pattern)
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

    bool createDirectory(const std::string& dirname) {
        boost::filesystem::path dir(dirname);
        if (boost::filesystem::create_directory(dir)) {
            return true;
        }

        return false;
    }

}}  // namespace leatherman::file_util
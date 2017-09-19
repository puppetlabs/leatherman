#include <leatherman/file_util/file.hpp>
#include <boost/nowide/fstream.hpp>
#include <boost/nowide/cenv.hpp>
#include <boost/filesystem.hpp>
#include <sstream>
#include <leatherman/logging/logging.hpp>
#include <leatherman/locale/locale.hpp>

// Mark string for translation (alias for leatherman::locale::format)
using leatherman::locale::_;

using namespace std;

namespace leatherman { namespace file_util {

namespace boost_error = boost::system::errc;
namespace boost_file = boost::filesystem;

    bool each_line(string const& path, function<bool(string &)> callback) {
        boost::nowide::ifstream in(path.c_str());
        if (!in) {
            return false;
        }

        string line;
        while (getline(in, line)) {
            if (!callback(line)) {
                break;
            }
        }
        return true;
    }

    string read(string const& path) {
        string contents;
        if (!read(path, contents)) {
            return {};
        }
        return contents;
    }

    bool read(string const& path, string& contents) {
        boost::nowide::ifstream in(path.c_str(), ios::in | ios::binary);
        ostringstream buffer;
        if (!in) {
            return false;
        }
        buffer << in.rdbuf();
        contents = buffer.str();
        return true;
    }

    bool file_readable(const std::string &file_path) {
        bool exists { false };
        if (file_path.empty()) {
            LOG_WARNING("file path is an empty string");
        } else {
            boost::system::error_code ec;
            boost_file::file_status status = boost_file::status(file_path.c_str(), ec);
            if (boost_file::exists(status) && !boost_file::is_directory(status)) {
                boost::nowide::ifstream file_stream(file_path.c_str());
                exists = file_stream.good();
                file_stream.close();
            } else {
                LOG_DEBUG("Error reading file: {1}", ec.message());
                exists = false;
            }
        }
        return exists;
    }

    void atomic_write_to_file(const std::string &text,
                              const std::string &file_path,
                              std::ios_base::openmode mode) {
        atomic_write_to_file(text, file_path, {}, mode);
    }

    void atomic_write_to_file(const std::string &text,
                              const std::string &file_path,
                              boost::optional<boost_file::perms> perms,
                              std::ios_base::openmode mode) {
        boost::nowide::ofstream ofs;
        std::string tmp_name = file_path + "~";
        ofs.open(tmp_name.c_str(), mode);
        if (!ofs.is_open()) {
            throw boost_file::filesystem_error { _("failed to open {1}", file_path),
                                                        boost_error::make_error_code(
                                                                boost_error::io_error) };
        }

        if (perms) {
            boost_file::permissions(tmp_name, *perms);
        }

        ofs << text;
        ofs.close();
        boost_file::rename(tmp_name.data(), file_path.data());
    }

    std::string tilde_expand(std::string path) {
        if (path[0] == '~' && (path.size() == 1 || path[1] == '/')) {
            auto result = get_home_path();
            result.append(path.begin() + 1, path.end());
            return result;
        }
        return path;
    }

    std::string shell_quote(std::string path) {
        std::stringstream ss;
        ss << boost::io::quoted(path);
        return ss.str();
    }

    std::string get_home_path() {
        #ifdef _WIN32
            auto home_var = "USERPROFILE";
            auto result = boost::nowide::getenv(home_var);
        #else
            auto home_var = "HOME";
            auto result = boost::nowide::getenv(home_var);
        #endif
        if (result){
            return result;
        } else {
            LOG_WARNING("{1} has not been set", home_var);
            return "";
        }
    }

}}  // namespace leatherman::file_util

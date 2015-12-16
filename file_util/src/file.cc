#include <leatherman/file_util/file.hpp>
#include <boost/nowide/fstream.hpp>
#include <boost/nowide/cenv.hpp>
#include <boost/filesystem.hpp>
#include <sstream>
#include <leatherman/logging/logging.hpp>

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

    // This function attempts to read a file at the given path in text mode and return its contents as
    // a string. If the file at the path is a symlink, it will be followed until a non-symlink file is
    // found. If anything goes wrong, a file_error will be thrown.
    std::string read(const boost_file::path& path) {
      boost_file::path p(path);
      while (boost_file::is_symlink(p)) {
        p = boost_file::read_symlink(p);
      }

      boost::nowide::ifstream file(p.c_str());
      if (!file.is_open()) {
        auto msg = boost::str(boost::format("Unable to open file %1% for reading: %2%")
            % p.string() % strerror(errno));
        throw file_error(msg);
      }

      std::stringstream contents_stream(std::ios_base::out);
      contents_stream << file.rdbuf();

      if (file.fail()) {
        auto msg = boost::str(boost::format("An I/O error occurred while reading the file %1%: %2%")
            % p.string() % strerror(errno));
        throw file_error(msg);
      }

      return contents_stream.str();
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
                LOG_DEBUG("Error reading file: %1%", ec.message());
                exists = false;
            }
        }
        return exists;
    }

    void atomic_write_to_file(const std::string &text,
                              const std::string &file_path,
                              std::ios_base::openmode mode) {
        boost::nowide::ofstream ofs;
        std::string tmp_name = file_path + "~";
        ofs.open(tmp_name.c_str(), mode);
        if (!ofs.is_open()) {
            throw boost_file::filesystem_error { "failed to open " + file_path,
                                                        boost_error::make_error_code(
                                                                boost_error::io_error) };
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
            LOG_WARNING("%1% has not been set", home_var);
            return "";
        }
    }

}}  // namespace leatherman::file_util

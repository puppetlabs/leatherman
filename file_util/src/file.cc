#include <leatherman/file_util/file.hpp>
#include <boost/filesystem.hpp>
#include <boost/nowide/fstream.hpp>
#include <sstream>
#include <leatherman/logging/logging.hpp>

#include <wordexp.h>

using namespace std;

namespace leatherman { namespace file_util {

    bool file::each_line(string const& path, function<bool(string&)> callback)
    {
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

    string file::read(string const& path)
    {
        string contents;
        if (!read(path, contents)) {
            return {};
        }
        return contents;
    }

    bool file::read(string const& path, string& contents)
    {
        boost::nowide::ifstream in(path.c_str(), ios::in | ios::binary);
        ostringstream buffer;
        if (!in) {
            return false;
        }
        buffer << in.rdbuf();
        contents = buffer.str();
        return true;
    }

    std::string shellExpand(std::string txt) {
        // This will store the expansion outcome
        wordexp_t result;

        // Expand and check the success
        if (wordexp(txt.c_str(), &result, 0) != 0 || result.we_wordc == 0) {
            wordfree(&result);
            return "";
        }

        // Get the expanded text and free the memory
        std::string expanded_txt { result.we_wordv[0] };
        wordfree(&result);
        return expanded_txt;
    }

    bool fileExists(const std::string& file_path) {
        bool exists { false };
        if (file_path.empty()) {
            LOG_WARNING("file path is an empty string");
        } else {
            std::ifstream file_stream { file_path };
            exists = file_stream.good();
            file_stream.close();
        }
        return exists;
    }

    void removeFile(const std::string& file_path) {
        if (file_util::fileExists(file_path)) {
            if (std::remove(file_path.c_str()) != 0) {
                throw file_error { "failed to remove " + file_path };
            }
        }
    }

    void streamToFile(const std::string& text,
                      const std::string& file_path,
                      std::ios_base::openmode mode) {
        std::ofstream ofs;
        std::string tmp_name = file_path + "~";
        ofs.open(tmp_name, mode);
        if (!ofs.is_open()) {
            throw file_error { "failed to open " + file_path };
        }
        ofs << text;
        ofs.close();
        rename(tmp_name.data(), file_path.data());
    }

    void writeToFile(const std::string& text, const std::string& file_path) {
        streamToFile(text, file_path, std::ofstream::out | std::ofstream::trunc);
    }

    std::string readFileAsString(std::string path) {
        std::string content = "";

        if (!fileExists(path)) {
            return content;
        }

        std::ifstream file { path };
        std::string buffer;

        while (std::getline(file, buffer)) {
            content += buffer;
            content.push_back('\n');
        }

        return content;
    }

}}  // namespace leatherman::file_util

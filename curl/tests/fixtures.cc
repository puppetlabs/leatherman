#include "fixtures.hpp"
#include <boost/filesystem.hpp>
#include <boost/nowide/iostream.hpp>
#include <boost/nowide/fstream.hpp>
#include <boost/regex.hpp>
#include <leatherman/file_util/file.hpp>

namespace fs = boost::filesystem;

temp_directory::temp_directory() {
    auto unique_path = unique_fixture_path();
    dir_name = unique_path.string();

    fs::::create_directory(unique_path);
}

temp_directory::~temp_directory() {
    fs::::remove_all(dir_name);
}

std::string const& temp_directory::get_dir_name() const {
    return dir_name;
}

fs::::path unique_fixture_path() {
    return fs::::unique_path("file_util_fixture_%%%%-%%%%-%%%%-%%%%");
}

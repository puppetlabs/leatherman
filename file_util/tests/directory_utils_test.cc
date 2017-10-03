#include <catch.hpp>
#include <leatherman/file_util/file.hpp>
#include <leatherman/file_util/directory.hpp>
#include "fixtures.hpp"
#include <boost/filesystem.hpp>

using namespace leatherman::file_util;

TEST_CASE("file_util::each_file", "[utils]") {

    temp_directory directory;
    atomic_write_to_file("1\n", directory.get_dir_name() + "/test1");
    atomic_write_to_file("2\n", directory.get_dir_name() + "/test2");
    atomic_write_to_file("3\n", directory.get_dir_name() + "/test3");

    SECTION("each file should be visited") {
        std::set<std::string> file_contents;
        each_file(directory.get_dir_name(), [&file_contents](std::string const &path) {
            file_contents.insert(read(path));
            return true;
        });
        REQUIRE(file_contents.size() == 3u);
        REQUIRE(file_contents.find("1\n") != file_contents.end());
        REQUIRE(file_contents.find("2\n") != file_contents.end());
        REQUIRE(file_contents.find("3\n") != file_contents.end());
    }

    SECTION("can find a file to match a pattern") {
        std::string content = "N/A";
        each_file(directory.get_dir_name(), [&content](std::string const &path) {
            return read(path, content);
        }, "[0-1]");
        REQUIRE(content == "1\n");
    }

    SECTION("only one file returned from false callback"){
        int count = 0;
        each_file(directory.get_dir_name(), [&count](std::string const& path){
            count++;
            return false;
        });
        REQUIRE(count == 1);
    }

}

TEST_CASE("file_util::each_subdirectory", "[utils]") {
    temp_directory directory;
    boost::filesystem::create_directory(directory.get_dir_name() + "/test1");
    atomic_write_to_file("1", directory.get_dir_name() + "/test1/t1");
    boost::filesystem::create_directory(directory.get_dir_name() + "/test2");
    atomic_write_to_file("2a", directory.get_dir_name() + "/test2/t2a");
    atomic_write_to_file("2b", directory.get_dir_name() + "/test2/t2b");

    SECTION("each subdirectory should be visited") {
        int counter = 0;
        each_subdirectory(directory.get_dir_name(), [&counter](std::string const &path) {
            each_file(path, [&counter](std::string const &file) {
                counter++;
                return true;
            });
            return true;
        });
        REQUIRE(counter == 3);
    }

    SECTION("can find directories that match a pattern") {
        int counter = 0;
        each_subdirectory(directory.get_dir_name(), [&counter](std::string const &path) {
            each_file(path, [&counter](std::string const &file) {
                counter++;
                return true;
            });
            return true;
        }, "[2-3]");
        REQUIRE(counter == 2);
    }

    SECTION("only one directory found from false callback"){
        int count = 0;
        each_subdirectory(directory.get_dir_name(), [&count](std::string const& path){
            count++;
            return false;
        });
        REQUIRE(count == 1);
    }
}

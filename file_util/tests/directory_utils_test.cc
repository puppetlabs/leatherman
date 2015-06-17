#include <catch.hpp>
#include <leatherman/file_util/file.hpp>
#include <leatherman/file_util/directory.hpp>
#include "fixtures.hpp"
#include <boost/filesystem.hpp>

namespace leatherman { namespace file_util {

    TEST_CASE("file_util::each_file", "[utils]") {

        temp_directory directory;
        atomic_write_to_file("1\n", directory.get_dir_name() + "/test1");
        atomic_write_to_file("2\n", directory.get_dir_name() + "/test2");
        atomic_write_to_file("3\n", directory.get_dir_name() + "/test3");

        SECTION("each file should be visited") {
            int ind = 0;
            each_file(directory.get_dir_name(), [&ind](std::string const &path) {
                ind++;
                return read(path) == (std::to_string(ind) + "\n");
            });
            REQUIRE(ind == 3);
        }

        SECTION("can find a file to match a pattern") {
            std::string content = "N/A";
            each_file(directory.get_dir_name(), [&content](std::string const &path) {
                return read(path, content);
            }, "1");
            REQUIRE(content == "1\n");
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
            }, "2");
            REQUIRE(counter == 2);
        }
    }

}}  //namespace leatherman::file_util
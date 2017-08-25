#include <catch.hpp>
#include <leatherman/file_util/file.hpp>
#include "fixtures.hpp"
#include <boost/filesystem.hpp>

#ifdef _WIN32
#include <windows.h>
#endif

using namespace leatherman::file_util;

TEST_CASE("file_util::tilde_expand", "[utils]") {

#ifdef _WIN32
    _putenv("USERPROFILE=/testhome");
#else
    setenv("HOME", "/testhome", 1);
#endif

    SECTION("empty path should be empty") {
        REQUIRE(tilde_expand("") == "");
    }

    SECTION("spaces should be preserved") {
        REQUIRE(tilde_expand("i like spaces") == "i like spaces");
    }

    SECTION("should expand using environment variable") {
        CHECK(tilde_expand("~") == "/testhome");
        CHECK(tilde_expand("~/") == "/testhome/");
        CHECK(tilde_expand("~/foo") == "/testhome/foo");
    }

    SECTION("only a ~ at the start") {
        REQUIRE(tilde_expand("/foo/bar~") == "/foo/bar~");
    }

    SECTION("~baz/foo does not expand") {
        REQUIRE(tilde_expand("~baz/foo") == "~baz/foo");
    }

    SECTION("it should expand the home directory path") {
        REQUIRE(tilde_expand("~/foo") != "~/foo");
    }

    SECTION("it should not expand the working directory path") {
        REQUIRE(tilde_expand("./foo") == "./foo");
    }

    auto home_path = get_home_path();

    SECTION("it should expand ~ to the HOME env var") {
        REQUIRE(tilde_expand("~") == home_path);
    }

    SECTION("it should expand ~ as the base directory") {
        std::string expected_path{home_path + "/spam"};
        std::string expanded_path{tilde_expand("~/spam")};
        REQUIRE(expanded_path == expected_path);
    }
}

TEST_CASE("shell_quote", "[utils]") {
    SECTION("empty string") {
        REQUIRE(shell_quote("") == "\"\"");
    }

    SECTION("single word") {
        REQUIRE(shell_quote("plain") == "\"plain\"");
    }

    SECTION("words separated by space") {
        REQUIRE(shell_quote("a space") == "\"a space\"");
    }

    SECTION("exclamation mark") {
        REQUIRE(shell_quote("!csh") == "\"!csh\"");
    }

    SECTION("single quote before expression") {
        REQUIRE(shell_quote("'open quote") == "\"'open quote\"");
    }

    SECTION("single quote after expression") {
        REQUIRE(shell_quote("close quote'") == "\"close quote'\"");
    }

    SECTION("double quote before expression") {
        REQUIRE(shell_quote("\"open doublequote")
                == "\"\\\"open doublequote\"");
    }

    SECTION("double quote after expression") {
        REQUIRE(shell_quote("close doublequote\"")
                == "\"close doublequote\\\"\"");
    }
}

TEST_CASE("lth_file::file_readable", "[utils]") {
    SECTION("it can check that a file does not exist") {
        auto file_path = unique_fixture_path().string();
        CAPTURE(file_path);
        REQUIRE_FALSE(file_readable(file_path));
    }

    SECTION("directories are not readable") {
        temp_directory dir_path;
        REQUIRE_FALSE(file_readable(dir_path.get_dir_name()));
    }
}

TEST_CASE("lth_file::atomic_write_to_file", "[utils]") {
    SECTION("it can write to a regular file, ensure it exists, and delete it") {
        auto file_path = unique_fixture_path().string();
        REQUIRE_FALSE(file_readable(file_path));
        atomic_write_to_file("test\n", file_path);
        REQUIRE(file_readable(file_path));
        boost::filesystem::remove(file_path);
        REQUIRE_FALSE(file_readable(file_path));
    }

    SECTION("can write to an existing file") {
        temp_file file("existing file");
        REQUIRE(file_readable(file.get_file_name()));
        atomic_write_to_file("test", file.get_file_name());
        REQUIRE(file_readable(file.get_file_name()));
        REQUIRE(read(file.get_file_name()) == "test");
    }

#ifndef _WIN32
    SECTION("can write a file with permissions") {
        auto file_path = unique_fixture_path().string();
        REQUIRE_FALSE(file_readable(file_path));
        auto perms = boost::filesystem::owner_read | boost::filesystem::owner_write;
        atomic_write_to_file("test\n", file_path, perms, std::ios::binary);
        REQUIRE(file_readable(file_path));
        auto stat = boost::filesystem::status(file_path);
        REQUIRE(stat.permissions() == perms);
        boost::filesystem::remove(file_path);
        REQUIRE_FALSE(file_readable(file_path));
    }
#endif
}

TEST_CASE("file_util::read", "[utils]") {

    SECTION("trying to read a nonexistent file returns the empty string"){
        std::string contents;
        REQUIRE(read("does_not_exist") == "");
        REQUIRE_FALSE(read("does_not_exist", contents));
        REQUIRE(contents.empty());
    }

    SECTION("it can read from a file") {
        auto file_path = unique_fixture_path().string();
        atomic_write_to_file("test\n", file_path);
        REQUIRE(file_readable(file_path));
        std::string contents;
        REQUIRE(read(file_path, contents));
        REQUIRE(contents == "test\n");
        REQUIRE(read(file_path) == "test\n");
        boost::filesystem::remove(file_path);
    }
}

TEST_CASE("file_util::each_line", "[utils]") {

    SECTION("trying to read a nonexistent file returns false") {
        REQUIRE_FALSE(each_line("does_not_exist", [](std::string &line) {
            FAIL("should not be called");
            return true;
        }));
    }

    SECTION("an action is performed on each line of a file") {
        temp_file file("test1\ntest2\ntest3\n");
        int i = 0;
        REQUIRE(each_line(file.get_file_name(), [&i](std::string const &line) {
            i++;
            return line == ("test" + std::to_string(i));
        }));
        REQUIRE(i == 3);
    }

    SECTION("a callback that returns false stops at the first line"){
        temp_file file("test1\ntest2\ntest3\n");
        std::vector<std::string> lines;
        REQUIRE(each_line(file.get_file_name(), [&](std::string& line) {
            lines.emplace_back(move(line));
            return false;
        }));
        REQUIRE(lines.size() == 1u);
        REQUIRE(lines[0] == "test1");
    }
}

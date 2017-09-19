#include <catch.hpp>
#include <leatherman/execution/execution.hpp>
#include <leatherman/util/strings.hpp>
#include <leatherman/util/scope_exit.hpp>
#include <leatherman/util/scoped_env.hpp>
#include <leatherman/windows/windows.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/filesystem.hpp>
#include <boost/nowide/fstream.hpp>
#include <leatherman/util/regex.hpp>
#include "../fixtures.hpp"
#include "../log_capture.hpp"
#include "../lth_cat.hpp"
#include <stdlib.h>

using namespace std;
using namespace leatherman::util;
using namespace leatherman::execution;
using namespace leatherman::logging;
using namespace leatherman::execution::testing;
using leatherman::util::scoped_env;
using namespace boost::filesystem;

// Ruby doesn't appear to normalize commands passed to cmd.exe, so neither do we. A utility is provided
// here for familiarity in writing unit tests.
static string normalize(const char *filepath)
{
    return path(filepath).make_preferred().string();
}

SCENARIO("searching for programs with execution::which") {
    GIVEN("an absolute path") {
        THEN("the same path should be returned") {
            REQUIRE(which(EXEC_TESTS_DIRECTORY "/fixtures/windows/facts.bat")
                    == EXEC_TESTS_DIRECTORY "/fixtures/windows/facts.bat");
        }
    }
    GIVEN("a relative path") {
        THEN("it should find a file with the same relative offset from a directory on PATH") {
            REQUIRE(which("windows/facts", { EXEC_TESTS_DIRECTORY "/fixtures" })
                    == EXEC_TESTS_DIRECTORY "/fixtures\\windows/facts.bat");
        }
    }
    GIVEN("a file without an extension") {
        THEN("it should find a batch file with the same base name") {
            REQUIRE(which("facts", { EXEC_TESTS_DIRECTORY "/fixtures/windows" })
                    == EXEC_TESTS_DIRECTORY "/fixtures/windows\\facts.bat");
        }
    }
    GIVEN("a file that does not exist") {
        THEN("an empty string should be returned") {
            REQUIRE(which("not_on_the_path") == "");
        }
    }
     GIVEN("a file that exists but is not an executable") {
        THEN("an empty string should be returned") {
            REQUIRE(which("not_executable", { EXEC_TESTS_DIRECTORY "/fixtures/windows" }) == "");
        }
    }
}

SCENARIO("expanding command paths with execution::expand_command") {
    GIVEN("an executable on the PATH") {
        THEN("the executable is expanded to an absolute path") {
            REQUIRE(expand_command("facts 1 2 3", { EXEC_TESTS_DIRECTORY "/fixtures/windows" })
                    == EXEC_TESTS_DIRECTORY "/fixtures/windows\\facts.bat 1 2 3");
        }
    }
    GIVEN("a single-quoted command") {
        THEN("the expanded path should be single-quoted") {
            REQUIRE(expand_command("'facts' 1 2 3", { EXEC_TESTS_DIRECTORY "/fixtures/windows" })
                    == "'" EXEC_TESTS_DIRECTORY "/fixtures/windows\\facts.bat' 1 2 3");
        }
    }
    GIVEN("a double-quoted command") {
        THEN("the expanded path should be double-quoted") {
            REQUIRE(expand_command("\"facts\" 1 2 3", { EXEC_TESTS_DIRECTORY "/fixtures/windows" })
                    == "\"" EXEC_TESTS_DIRECTORY "/fixtures/windows\\facts.bat\" 1 2 3");
        }
    }
    GIVEN("a command not on PATH") {
        THEN("the returned command is empty") {
            REQUIRE(expand_command("not_on_the_path") == "");
        }
    }
    GIVEN("a non-executable command on PATH") {
        THEN("the returned command is empty") {
            REQUIRE(expand_command("not_executable", { EXEC_TESTS_DIRECTORY "/fixtures/windows" }) == "");
        }
    }
}

SCENARIO("executing commands with execution::execute") {
    auto get_variables = [](string const& input) {
        map<string, string> variables;
        leatherman::util::each_line(input, [&](string& line) {
            vector<string> parts;
            boost::split(parts, line, boost::is_any_of("="), boost::token_compress_off);
            if (parts.size() != 2u) {
                return true;
            }
            variables.emplace(make_pair(move(parts[0]), move(parts[1])));
            return true;
        });
        return variables;
    };
    std::string spool_dir { EXEC_TESTS_DIRECTORY "/spool" };
    auto get_file_content = [spool_dir](string const& filename) {
        string filepath((path(spool_dir) / filename).string());
        boost::nowide::ifstream strm(filepath.c_str());
        if (!strm) FAIL("failed to open file: " + filename);
        string content((istreambuf_iterator<char>(strm)), (istreambuf_iterator<char>()));
        strm.close();
        return content;
    };
    if (!exists(spool_dir) && !create_directories(spool_dir)) {
        FAIL("failed to create spool directory");
    }
    scope_exit spool_cleaner([spool_dir]() { remove_all(spool_dir); });
    GIVEN("a command that succeeds") {
        THEN("the output should be returned") {
            auto exec = execute("cmd.exe", { "/c", "type", normalize(EXEC_TESTS_DIRECTORY "/fixtures/ls/file3.txt") });
            REQUIRE(exec.success);
            REQUIRE(exec.output == "file3");
            REQUIRE(exec.error == "");
            REQUIRE(exec.exit_code == 0);
        }
        WHEN("the create new process group option is used") {
            auto exec = execute("cmd.exe", { "/c", "type", normalize(EXEC_TESTS_DIRECTORY "/fixtures/ls/file3.txt") }, 0, { execution_options::trim_output, execution_options::merge_environment, execution_options::redirect_stderr_to_null, execution_options::create_detached_process });
            REQUIRE(exec.success);
            REQUIRE(exec.output == "file3");
            REQUIRE(exec.error == "");
            REQUIRE(exec.exit_code == 0);
        }
        WHEN("expecting input") {
            auto exec = execute("cmd.exe", { "/c", CMAKE_BIN_DIRECTORY "/lth_cat.exe" }, "hello");
            REQUIRE(exec.success);
            REQUIRE(exec.output == "hello");
            REQUIRE(exec.error == "");
            REQUIRE(exec.exit_code == 0);
        }
        WHEN("expecting input with lots of output") {
            auto exec = execute("cmd.exe", { "/c", CMAKE_BIN_DIRECTORY "/lth_cat.exe", "prefix", "suffix", "overwhelm", "stderr" },
                "hello\ngoodbye", 0, { execution_options::merge_environment });
            REQUIRE(exec.success);
            auto expected = lth_cat::prefix+lth_cat::overwhelm+"hello\n"+lth_cat::overwhelm+"goodbye\n"+lth_cat::overwhelm+lth_cat::suffix;
            boost::replace_all(expected, "\n", "\r\n");
            REQUIRE(exec.output == expected);
            REQUIRE(exec.error == "hello\r\ngoodbye\r\n");
            REQUIRE(exec.exit_code == 0);
        }
        WHEN("requested to write stdout to file") {
            string out_file(spool_dir + "/stdout_test.out");
            auto exec = execute(EXEC_TESTS_DIRECTORY "/fixtures/windows/error_message.bat", {}, "", out_file);
            REQUIRE(exists(out_file));
            THEN("stdout is correctly redirected to file") {
                auto output = get_file_content("stdout_test.out");
                REQUIRE(output == "foo=bar\nsome more stuff\n");
            }
            THEN("the returned results are correct and stdout was not buffered") {
                REQUIRE(exec.success);
                REQUIRE(exec.output.empty());
                REQUIRE(exec.error == "error message!");
            }
        }
        WHEN("requested to write stdout and stderr to the same file with trim") {
            string out_file(spool_dir + "/stdout_stderr_test.out");
            auto exec = execute(EXEC_TESTS_DIRECTORY "/fixtures/windows/error_message.bat", {}, "", out_file, "", {}, nullptr, 0, { execution_options::trim_output, execution_options::merge_environment, execution_options::redirect_stderr_to_stdout });
            REQUIRE(boost::filesystem::exists(out_file));
            THEN("stdout and stderr are correctly redirected to file") {
                auto output = get_file_content("stdout_stderr_test.out");
                REQUIRE(output == "error message!\nfoo=bar\nsome more stuff\n");
            }
            THEN("the returned results are correct and out/err streams were not buffered") {
                REQUIRE(exec.success);
                REQUIRE(exec.output.empty());
                REQUIRE(exec.error.empty());
            }
        }
        WHEN("requested to write stdout to a file in an unknown directory") {
            bool success = false;
            try {
                execute("cmd.exe", { "/c", CMAKE_BIN_DIRECTORY "/lth_cat.exe" }, "spam", EXEC_TESTS_DIRECTORY "/spam/eggs/stdout.out");
                success = true;
            } catch (...) {
                // pass
            }
            THEN("it fails") {
                REQUIRE_FALSE(success);
            }
        }
        WHEN("requested to write both stdout and stderr to file without trim") {
            string out_file(spool_dir + "/stdout_test_b.out");
            string err_file(spool_dir + "/stderr_test_b.err");
            auto exec = execute(EXEC_TESTS_DIRECTORY "/fixtures/windows/error_message.bat", {}, "",
                                out_file, err_file, {}, nullptr, 0, lth_util::option_set<execution_options>{});
            REQUIRE(boost::filesystem::exists(out_file));
            REQUIRE(boost::filesystem::exists(err_file));
            THEN("stdout and stderr are correctly redirected to different files") {
                auto output = get_file_content("stdout_test_b.out");
                auto error = get_file_content("stderr_test_b.err");
                REQUIRE(output == "foo=bar\n\nsome more stuff\n");
                REQUIRE(error == "error message!\n");
            }
            THEN("the returned results are correct and out/err streams were not buffered") {
                REQUIRE(exec.success);
                REQUIRE(exec.output.empty());
                REQUIRE(exec.error.empty());
            }
        }
        WHEN("requested to execute a PID callback") {
            int pid_from_callback = 0;
            auto exec = execute(EXEC_TESTS_DIRECTORY "/fixtures/windows/error_message.bat", {}, "", {}, [&pid_from_callback](size_t pid) { pid_from_callback = pid; });
            THEN("the returned results are correct") {
                REQUIRE(exec.success);
                REQUIRE(exec.output == "foo=bar\r\n\r\nsome more stuff");
                REQUIRE(exec.error.empty());  // stderr is redirected to null
            }
            THEN("the callback is successfully executed") {
                REQUIRE(pid_from_callback > 0);
            }
        }
    }
    GIVEN("a command that fails") {
        WHEN("default options are used") {
            auto exec = execute("cmd.exe", { "/c", "dir", "/B", "does_not_exist" });
            THEN("no output is returned") {
                REQUIRE_FALSE(exec.success);
                REQUIRE(exec.output == "");
                REQUIRE(exec.error == "");
                REQUIRE(exec.exit_code > 0);
            }
        }
        WHEN("the create new process group option is used") {
            auto exec = execute("cmd.exe", { "/c", "dir", "/B", "does_not_exist" }, 0, { execution_options::trim_output, execution_options::merge_environment, execution_options::redirect_stderr_to_null, execution_options::create_detached_process });
            THEN("no output is returned") {
                REQUIRE_FALSE(exec.success);
                REQUIRE(exec.output == "");
                REQUIRE(exec.error == "");
                REQUIRE(exec.exit_code > 0);
            }
        }
        WHEN("the redirect stderr option is used") {
            auto exec = execute("cmd.exe", { "/c", "dir", "/B", "does_not_exist" }, 0, { execution_options::trim_output, execution_options::merge_environment, execution_options::redirect_stderr_to_stdout });
            THEN("error output is returned on stdout") {
                REQUIRE_FALSE(exec.success);
                REQUIRE(exec.output == "File Not Found");
                REQUIRE(exec.error == "");
                REQUIRE(exec.exit_code > 0);
            }
        }
        WHEN("not redirecting stderr to null") {
            auto exec = execute("cmd.exe", { "/c", "dir", "/B", "does_not_exist" }, 0, { execution_options::trim_output, execution_options::merge_environment });
            THEN("error output is returned") {
                REQUIRE_FALSE(exec.success);
                REQUIRE(exec.output == "");
                REQUIRE(exec.error == "File Not Found");
                REQUIRE(exec.exit_code > 0);
            }
        }
        WHEN("the 'throw on non-zero exit' option is used") {
            THEN("a child exit exception is thrown") {
                REQUIRE_THROWS_AS(execute("cmd.exe", { "/c", "dir", "/B", "does_not_exist" }, 0, { execution_options::trim_output, execution_options::merge_environment, execution_options::throw_on_nonzero_exit }), child_exit_exception);
            }
        }
        WHEN("requested to merge the environment") {
            scoped_env test_var("TEST_INHERITED_VARIABLE", "TEST_INHERITED_VALUE");
            auto exec = execute("cmd.exe", { "/c", "set" }, { { "TEST_VARIABLE1", "TEST_VALUE1" }, { "TEST_VARIABLE2", "TEST_VALUE2" } });
            REQUIRE(exec.success);
            REQUIRE(exec.error == "");
            auto variables = get_variables(exec.output);
            THEN("the child environment should contain the given variables") {
                REQUIRE(variables.size() > 4u);
                REQUIRE(variables.count("TEST_VARIABLE1") == 1u);
                REQUIRE(variables["TEST_VARIABLE1"] == "TEST_VALUE1");
                REQUIRE(variables.count("TEST_VARIABLE1") == 1u);
                REQUIRE(variables["TEST_VARIABLE1"] == "TEST_VALUE1");
            }
            THEN("the child environment should have LC_ALL and LANG set to C") {
                REQUIRE(variables.count("LC_ALL") == 1u);
                REQUIRE(variables["LC_ALL"] == "C");
                REQUIRE(variables.count("LANG") == 1u);
                REQUIRE(variables["LANG"] == "C");
            }
        }
        WHEN("requested to override the environment") {
            scoped_env test_var("TEST_INHERITED_VARIABLE", "TEST_INHERITED_VALUE");
            auto exec = execute("cmd.exe", { "/c", "set" }, { { "TEST_VARIABLE1", "TEST_VALUE1" }, { "TEST_VARIABLE2", "TEST_VALUE2" } }, 0, { execution_options::trim_output });
            REQUIRE(exec.success);
            REQUIRE(exec.error == "");
            auto variables = get_variables(exec.output);
            THEN("the child environment should only contain the given variables") {
                REQUIRE(variables.count("TEST_VARIABLE1") == 1u);
                REQUIRE(variables["TEST_VARIABLE1"] == "TEST_VALUE1");
                REQUIRE(variables.count("TEST_VARIABLE1") == 1u);
                REQUIRE(variables["TEST_VARIABLE1"] == "TEST_VALUE1");
            }
            THEN("the child environment should have LC_ALL and LANG set to C") {
                REQUIRE(variables.count("LC_ALL") == 1u);
                REQUIRE(variables["LC_ALL"] == "C");
                REQUIRE(variables.count("LANG") == 1u);
                REQUIRE(variables["LANG"] == "C");
            }
        }
        WHEN("requested to override LC_ALL or LANG") {
            auto exec = execute("cmd.exe", { "/c", "set" }, { { "LANG", "FOO" }, { "LC_ALL", "BAR" } });
            REQUIRE(exec.success);
            REQUIRE(exec.error == "");
            auto variables = get_variables(exec.output);
            THEN("the values should be passed to the child process") {
                REQUIRE(variables.count("LC_ALL") == 1u);
                REQUIRE(variables["LC_ALL"] == "BAR");
                REQUIRE(variables.count("LANG") == 1u);
                REQUIRE(variables["LANG"] == "FOO");
            }
        }
    }
    GIVEN("a command that outputs leading/trailing whitespace") {
        THEN("whitespace should be trimmed by default") {
            auto exec = execute("cmd.exe", { "/c", "type", normalize(EXEC_TESTS_DIRECTORY "/fixtures/ls/file1.txt") });
            REQUIRE(exec.success);
            REQUIRE(exec.output == "this is a test of trimming");
            REQUIRE(exec.error == "");
        }
        WHEN("the 'trim whitespace' option is not used") {
            auto exec = execute("cmd.exe", { "/c", "type", normalize(EXEC_TESTS_DIRECTORY "/fixtures/ls/file1.txt") }, 0, { execution_options::merge_environment });
            REQUIRE(exec.success);
            THEN("whitespace should not be trimmed") {
                REQUIRE(exec.output == "   this is a test of trimming   ");
                REQUIRE(exec.error == "");
            }
        }
    }
    GIVEN("a long-running command") {
        WHEN("given a timeout") {
            THEN("a timeout exception should be thrown") {
                string ruby = which("ruby.exe");
                if (ruby.empty()) {
                    WARN("skipping command timeout test because no ruby was found on the PATH.");
                    return;
                }

                try {
                    execute("cmd.exe", { "/c", "ruby.exe", "-e", "sleep 60" }, 1);
                    FAIL("did not throw timeout exception");
                } catch (timeout_exception const& ex) {
                    // Verify the process was killed
                    REQUIRE(OpenProcess(0, FALSE, ex.pid()) == nullptr);
                } catch (exception const&) {
                    FAIL("unexpected exception thrown");
                }
            }
        }
    }
    GIVEN("stderr is redirected to null") {
        WHEN("using a debug log level") {
            log_capture capture(log_level::debug);
            auto exec = execute(EXEC_TESTS_DIRECTORY "/fixtures/windows/error_message.bat");
            REQUIRE(exec.success);
            REQUIRE(exec.output == "foo=bar\r\n\r\nsome more stuff");
            REQUIRE(exec.error.empty());
            THEN("stderr is logged") {
                auto output = capture.result();
                CAPTURE(output);
                REQUIRE(re_search(output, boost::regex("DEBUG !!! - error message!")));
            }
        }
        WHEN("not using a debug log level") {
            log_capture capture(log_level::warning);
            auto exec = execute(EXEC_TESTS_DIRECTORY "/fixtures/windows/error_message.bat");
            REQUIRE(exec.success);
            REQUIRE(exec.output == "foo=bar\r\n\r\nsome more stuff");
            REQUIRE(exec.error.empty());
            THEN("stderr is not logged") {
                auto output = capture.result();
                CAPTURE(output);
                REQUIRE_FALSE(re_search(output, boost::regex("DEBUG !!! - error message!")));
            }
        }
    }
    GIVEN("a command that outputs windows-style newlines") {
        // These are the default options so that I don't override them.
        lth_util::option_set<execution_options> options = { execution_options::trim_output, execution_options::merge_environment, execution_options::redirect_stderr_to_null };
        WHEN("newlines are not normalized") {
            auto exec = execute("cmd.exe", { "/c", "type", normalize(EXEC_TESTS_DIRECTORY "/fixtures/ls/crlf.txt") }, 0, options);
            REQUIRE(exec.success);
            REQUIRE(exec.error == "");
            REQUIRE(exec.output.find('\r') != std::string::npos);
        }
        WHEN("requested to normalize newlines") {
            auto exec = execute("cmd.exe", { "/c", "type", normalize(EXEC_TESTS_DIRECTORY "/fixtures/ls/crlf.txt") }, 0, options | option_set<execution_options>{ execution_options::convert_newlines });
            REQUIRE(exec.success);
            REQUIRE(exec.error == "");
            REQUIRE(exec.output.find('\r') == std::string::npos);
        }
    }
}

SCENARIO("executing commands with leatherman::execution::each_line") {
    GIVEN("a command that succeeds") {
        THEN("each line of output should be returned") {
            vector<string> lines;
            bool success = leatherman::execution::each_line(
                "cmd.exe",
                {
                    "/c",
                    "type",
                    normalize(EXEC_TESTS_DIRECTORY "/fixtures/ls/file4.txt")
                },
                [&](string& line) {
                    lines.push_back(line);
                    return true;
                });
            REQUIRE(success);
            REQUIRE(lines.size() == 4u);
            REQUIRE(lines[0] == "line1");
            REQUIRE(lines[1] == "line2");
            REQUIRE(lines[2] == "line3");
            REQUIRE(lines[3] == "line4");
        }
        WHEN("output stops when false is returned from callback") {
            vector<string> lines;
            bool success = leatherman::execution::each_line(
                "cmd.exe",
                {
                    "/c",
                    "type",
                    normalize(EXEC_TESTS_DIRECTORY "/fixtures/ls/file4.txt")
                },
                [&](string& line) {
                    lines.push_back(line);
                    return false;
                });
            REQUIRE(success);
            REQUIRE(lines.size() == 1u);
            REQUIRE(lines[0] == "line1");
        }
        WHEN("requested to merge the environment") {
            scoped_env test_var("TEST_INHERITED_VARIABLE", "TEST_INHERITED_VALUE");
            map<string, string> variables;
            bool success = each_line(
                "cmd.exe",
                {
                    "/c",
                    "set"
                },
                {
                    {"TEST_VARIABLE1", "TEST_VALUE1" },
                    {"TEST_VARIABLE2", "TEST_VALUE2" }
                },
                [&](string& line) {
                    vector<string> parts;
                    boost::split(parts, line, boost::is_any_of("="), boost::token_compress_off);
                    if (parts.size() != 2u) {
                        return true;
                    }
                    variables.emplace(make_pair(move(parts[0]), move(parts[1])));
                    return true;
                });
            REQUIRE(success);
            THEN("the child environment should contain the given variables") {
                REQUIRE(variables.size() > 4u);
                REQUIRE(variables.count("TEST_VARIABLE1") == 1u);
                REQUIRE(variables["TEST_VARIABLE1"] == "TEST_VALUE1");
                REQUIRE(variables.count("TEST_VARIABLE1") == 1u);
                REQUIRE(variables["TEST_VARIABLE1"] == "TEST_VALUE1");
            }
            THEN("the child environment should have LC_ALL and LANG set to C") {
                REQUIRE(variables.count("LC_ALL") == 1u);
                REQUIRE(variables["LC_ALL"] == "C");
                REQUIRE(variables.count("LANG") == 1u);
                REQUIRE(variables["LANG"] == "C");
            }
        }
        WHEN("requested to override the environment") {
            scoped_env test_var("TEST_INHERITED_VARIABLE", "TEST_INHERITED_VALUE");
            map<string, string> variables;
            bool success = each_line(
                "cmd.exe",
                {
                    "/c",
                    "set"
                },
                {
                    {"TEST_VARIABLE1", "TEST_VALUE1" },
                    {"TEST_VARIABLE2", "TEST_VALUE2" }
                },
                [&](string& line) {
                    vector<string> parts;
                    boost::split(parts, line, boost::is_any_of("="), boost::token_compress_off);
                    if (parts.size() != 2u) {
                        return true;
                    }
                    variables.emplace(make_pair(move(parts[0]), move(parts[1])));
                    return true;
                },
                nullptr,
                0,
                {
                    execution_options::trim_output
                });
            REQUIRE(success);
            THEN("the child environment should only contain the given variables") {
                // Windows adds several extra variables, such as COMSPEC, PATHEXT, and PROMPT.
                // Leave some buffer room for future additions, while ensuring we don't include
                // everything.
                REQUIRE(variables.size() < 10u);
                REQUIRE(variables.count("TEST_VARIABLE1") == 1u);
                REQUIRE(variables["TEST_VARIABLE1"] == "TEST_VALUE1");
                REQUIRE(variables.count("TEST_VARIABLE1") == 1u);
                REQUIRE(variables["TEST_VARIABLE1"] == "TEST_VALUE1");
                REQUIRE(variables.count("TEST_INHERITED_VARIABLE") == 0u);
            }
            THEN("the child environment should have LC_ALL and LANG set to C") {
                REQUIRE(variables.count("LC_ALL") == 1u);
                REQUIRE(variables["LC_ALL"] == "C");
                REQUIRE(variables.count("LANG") == 1u);
                REQUIRE(variables["LANG"] == "C");
            }
        }
        WHEN("requested to override LC_ALL or LANG") {
            map<string, string> variables;
            bool success = each_line(
                "cmd.exe",
                {
                    "/c",
                    "set"
                },
                {
                    {"LANG", "FOO" },
                    { "LC_ALL", "BAR" }
                },
                [&](string& line) {
                    vector<string> parts;
                    boost::split(parts, line, boost::is_any_of("="), boost::token_compress_off);
                    if (parts.size() != 2u) {
                        return true;
                    }
                    variables.emplace(make_pair(move(parts[0]), move(parts[1])));
                    return true;
                });
            REQUIRE(success);
            THEN("the values should be passed to the child process") {
                REQUIRE(variables.count("LC_ALL") == 1u);
                REQUIRE(variables["LC_ALL"] == "BAR");
                REQUIRE(variables.count("LANG") == 1u);
                REQUIRE(variables["LANG"] == "FOO");
            }
        }
        WHEN("requested to inherit locale") {
            scoped_env test_var("TEST_INHERITED_VARIABLE", "TEST_INHERITED_VALUE");
            scoped_env lc_all("LC_ALL", "en_US.UTF-8");
            scoped_env lang("LANG", "en_US.UTF-8");
            map<string, string> variables;
            bool success = each_line(
                "cmd.exe",
                {
                    "/c",
                    "set"
                },
                [&](string& line) {
                    vector<string> parts;
                    boost::split(parts, line, boost::is_any_of("="), boost::token_compress_off);
                    if (parts.size() != 2u) {
                        return true;
                    }
                    variables.emplace(make_pair(move(parts[0]), move(parts[1])));
                    return true;
                },
                nullptr,
                0,
                {
                    execution_options::trim_output,
                    execution_options::inherit_locale
                });
            REQUIRE(success);
            THEN("the child environment should only have LC_ALL and LANG set to en_US.UTF-8") {
                // Windows adds several extra variables, such as COMSPEC, PATHEXT, and PROMPT.
                // Leave some buffer room for future additions, while ensuring we don't include
                // everything.
                REQUIRE(variables.size() < 10u);
                REQUIRE(variables.count("LC_ALL") == 1u);
                REQUIRE(variables["LC_ALL"] == "en_US.UTF-8");
                REQUIRE(variables.count("LANG") == 1u);
                REQUIRE(variables["LANG"] == "en_US.UTF-8");
                REQUIRE(variables.count("TEST_INHERITED_VARIABLE") == 0u);
            }
        }
        WHEN("requested to inherit locale with no locale set") {
            scoped_env test_var("TEST_INHERITED_VARIABLE", "TEST_INHERITED_VALUE");
            scoped_env lc_all("LC_ALL");
            scoped_env lang("LANG");
            map<string, string> variables;
            bool success = each_line(
                "cmd.exe",
                {
                    "/c",
                    "set"
                },
                [&](string& line) {
                    vector<string> parts;
                    boost::split(parts, line, boost::is_any_of("="), boost::token_compress_off);
                    if (parts.size() != 2u) {
                        return true;
                    }
                    variables.emplace(make_pair(move(parts[0]), move(parts[1])));
                    return true;
                },
                nullptr,
                0,
                {
                    execution_options::trim_output,
                    execution_options::inherit_locale
                });
            REQUIRE(success);
            THEN("the child environment should only have LC_ALL and LANG set to en_US.UTF-8") {
                // Windows adds several extra variables, such as COMSPEC, PATHEXT, and PROMPT.
                // Leave some buffer room for future additions, while ensuring we don't include
                // everything.
                REQUIRE(variables.size() < 8u);
                REQUIRE(variables.count("LC_ALL") == 0u);
                REQUIRE(variables.count("LANG") == 0u);
                REQUIRE(variables.count("TEST_INHERITED_VARIABLE") == 0u);
            }
        }
        WHEN("requested to inherit locale with parent environment") {
            scoped_env test_var("TEST_INHERITED_VARIABLE", "TEST_INHERITED_VALUE");
            scoped_env lc_all("LC_ALL", "en_US.UTF-8");
            scoped_env lang("LANG", "en_US.UTF-8");
            map<string, string> variables;
            bool success = each_line(
                "cmd.exe",
                {
                    "/c",
                    "set"
                },
                [&](string& line) {
                    vector<string> parts;
                    boost::split(parts, line, boost::is_any_of("="), boost::token_compress_off);
                    if (parts.size() != 2u) {
                        return true;
                    }
                    variables.emplace(make_pair(move(parts[0]), move(parts[1])));
                    return true;
                },
                nullptr,
                0,
                {
                    execution_options::trim_output,
                    execution_options::merge_environment,
                    execution_options::inherit_locale
                });
            REQUIRE(success);
            THEN("the child environment should contain the merged variables") {
                REQUIRE(variables.size() > 3u);
                REQUIRE(variables.count("TEST_INHERITED_VARIABLE") == 1);
                REQUIRE(variables["TEST_INHERITED_VARIABLE"] == "TEST_INHERITED_VALUE");
            }
            THEN("the child environment should have LC_ALL and LANG set to en_US.UTF-8") {
                REQUIRE(variables.count("LC_ALL") == 1u);
                REQUIRE(variables["LC_ALL"] == "en_US.UTF-8");
                REQUIRE(variables.count("LANG") == 1u);
                REQUIRE(variables["LANG"] == "en_US.UTF-8");
            }
        }
    }
    GIVEN("a command that fails") {
        WHEN("default options are used") {
            THEN("no output is returned") {
                auto success = leatherman::execution::each_line(
                    "cmd.exe",
                    {
                        "/c",
                        "dir",
                        "/B",
                        "does_not_exist"
                    },
                    [](string& line) {
                        FAIL("should not be called");
                        return true;
                    });
                REQUIRE_FALSE(success);
            }
        }
        WHEN("the redirect stderr option is used") {
            string output;
            auto result = leatherman::execution::each_line(
                "cmd.exe",
                {
                    "/c",
                    "dir",
                    "/B",
                    "does_not_exist"
                },
                [&](string& line) {
                    if (!output.empty()) {
                        output += "\n";
                    }
                    output += line;
                    return true;
                },
                [&](string&) {
                    FAIL("should not be called");
                    return true;
                },
                0,
                {
                    execution_options::trim_output,
                    execution_options::merge_environment,
                    execution_options::redirect_stderr_to_stdout
                });
            THEN("error output is returned on stdout") {
                REQUIRE_FALSE(result);
                REQUIRE(output == "File Not Found");
            }
        }
        WHEN("not redirecting stderr to null") {
            string output;
            auto result = leatherman::execution::each_line(
                "cmd.exe",
                {
                    "/c",
                    "dir",
                    "/B",
                    "does_not_exist"
                },
                [&](string&) {
                    FAIL("should not be called.");
                    return true;
                },
                [&](string& line) {
                    if (!output.empty()) {
                        output += "\n";
                    }
                    output += line;
                    return true;
                });
            THEN("error output is returned") {
                REQUIRE_FALSE(result);
                REQUIRE(output == "File Not Found");
            }
        }
        WHEN("the 'throw on non-zero exit' option is used") {
            THEN("a child exit exception is thrown") {
                REQUIRE_THROWS_AS(each_line("cmd.exe", { "/c", "dir", "/B", "does_not_exist" }, nullptr, nullptr, 0, {execution_options::trim_output, execution_options::merge_environment, execution_options::throw_on_nonzero_exit}), child_exit_exception);
            }
        }
    }
    GIVEN("a long-running command") {
        WHEN("given a timeout") {
            THEN("a timeout exception should be thrown") {
                string ruby = which("ruby.exe");
                if (ruby.empty()) {
                    WARN("skipping command timeout test because no ruby was found on the PATH.");
                    return;
                }
                try {
                    each_line("cmd.exe", { "/c", "ruby.exe", "-e", "sleep 60" }, nullptr, nullptr, 1);
                    FAIL("did not throw timeout exception");
                } catch (timeout_exception const& ex) {
                    // Verify the process was killed
                    REQUIRE(OpenProcess(0, FALSE, ex.pid()) == nullptr);
                } catch (exception const&) {
                    FAIL("unexpected exception thrown");
                }
            }
        }
    }
    GIVEN("stderr is redirected to null") {
        WHEN("using a debug log level") {
            log_capture capture(log_level::debug);
            REQUIRE(leatherman::execution::each_line(EXEC_TESTS_DIRECTORY "/fixtures/windows/error_message.bat", nullptr));
            THEN("stderr is logged") {
                auto output = capture.result();
                CAPTURE(output);
                REQUIRE(re_search(output, boost::regex("DEBUG !!! - error message!")));
            }
        }
        WHEN("not using a debug log level") {
            log_capture capture(log_level::warning);
            REQUIRE(leatherman::execution::each_line(EXEC_TESTS_DIRECTORY "/fixtures/windows/error_message.bat", nullptr));
            THEN("stderr is not logged") {
                auto output = capture.result();
                CAPTURE(output);
                REQUIRE_FALSE(re_search(output, boost::regex("DEBUG !!! - error message!")));
            }
        }
    }
}

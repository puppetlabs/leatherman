#include <catch.hpp>
#include <leatherman/execution/execution.hpp>
#include <boost/algorithm/string.hpp>
#include <leatherman/util/regex.hpp>
#include <leatherman/util/strings.hpp>
#include "../fixtures.hpp"
#include "../log_capture.hpp"
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>

using namespace std;
using namespace leatherman::logging;
using namespace leatherman::execution::testing;
using namespace leatherman::util;
using namespace leatherman::execution;

SCENARIO("searching for programs with execution::which") {
    GIVEN("an absolute path to an executable file") {
        THEN("the same path should be returned") {
            REQUIRE(
                which(EXEC_TESTS_DIRECTORY "/fixtures/facts") ==
                EXEC_TESTS_DIRECTORY "/fixtures/facts"
            );
        }
    }
    GIVEN("an absolute path to a non-executable file") {
        THEN("an empty string should be returned") {
            REQUIRE(
                which(EXEC_TESTS_DIRECTORY "/fixtures/not_executable") ==
                ""
            );
        }
    }
    GIVEN("an absolute path to a non-existant file") {
        THEN("an empty string should be returned") {
            REQUIRE(
                which(EXEC_TESTS_DIRECTORY "/fixtures/does_not_exist") ==
                ""
            );
        }
    }
    GIVEN("an executable file on the PATH") {
        THEN("the full path should be returned") {
            REQUIRE(
                which("facts", { EXEC_TESTS_DIRECTORY "/fixtures/" }) ==
                EXEC_TESTS_DIRECTORY "/fixtures/facts"
            );
        }
    }
    GIVEN("a path relative to a directory on PATH") {
        THEN("the full path should be returned") {
            REQUIRE(
                which("facts", { EXEC_TESTS_DIRECTORY "/fixtures" }) ==
                EXEC_TESTS_DIRECTORY "/fixtures/facts"
            );
        }
    }
    GIVEN("a file that does not exist on PATH") {
        THEN("an empty string should be returned") {
            REQUIRE(
                which("not_on_the_path") ==
                ""
            );
        }
    }
    GIVEN("a file that is not executable") {
        THEN("an empty string should be returned") {
            REQUIRE(
                which("not_executable", { EXEC_TESTS_DIRECTORY "/fixtures" }) ==
                ""
            );
        }
    }
}

SCENARIO("expanding command paths with execution::expand_command") {
    GIVEN("an executable on the PATH") {
        THEN("the executable is expanded to an absolute path") {
            REQUIRE(
                expand_command("facts 1 2 3", { EXEC_TESTS_DIRECTORY "/fixtures" }) ==
                EXEC_TESTS_DIRECTORY "/fixtures/facts 1 2 3"
            );
        }
    }
    GIVEN("a single-quoted command") {
        THEN("the expanded path should be single-quoted") {
            REQUIRE(
                expand_command("'facts' 1 2 3", { EXEC_TESTS_DIRECTORY "/fixtures" }) ==
                "'" EXEC_TESTS_DIRECTORY "/fixtures/facts' 1 2 3"
            );
        }
    }
    GIVEN("a double-quoted command") {
        THEN("the expanded path should be double-quoted") {
            REQUIRE(
                expand_command("\"facts\" 1 2 3", { EXEC_TESTS_DIRECTORY "/fixtures" }) ==
                "\"" EXEC_TESTS_DIRECTORY "/fixtures/facts\" 1 2 3"
            );
        }
    }
    GIVEN("a command not on PATH") {
        THEN("the returned command is empty") {
            REQUIRE(expand_command("not_on_the_path") == "");
        }
    }
    GIVEN("a non-executable command on PATH") {
        THEN("the returned command is empty") {
            REQUIRE(expand_command("not_executable", { EXEC_TESTS_DIRECTORY "/fixtures" }) == "");
        }
    }
}

SCENARIO("executing commands with execution::execute") {
    auto get_variables = [](string const& input) {
        map<string, string> variables;
        leatherman::util::each_line(input, [&](string& line) {
            vector<string> parts;
            boost::split(parts, line, boost::is_any_of("="), boost::token_compress_off);
            if (parts.size() != 2) {
                return true;
            }
            variables.emplace(make_pair(move(parts[0]), move(parts[1])));
            return true;
        });
        return variables;
    };
    GIVEN("a command that succeeds") {
        THEN("the output should be returned") {
            auto exec = execute("cat", { EXEC_TESTS_DIRECTORY "/fixtures/ls/file3.txt" });
            REQUIRE(exec.success);
            REQUIRE(exec.output == "file3");
            REQUIRE(exec.exit_code == 0);
        }
        WHEN("requested to merge the environment") {
            setenv("TEST_INHERITED_VARIABLE", "TEST_INHERITED_VALUE", 1);
            auto exec = execute("env", {}, { {"TEST_VARIABLE1", "TEST_VALUE1" }, {"TEST_VARIABLE2", "TEST_VALUE2" } });
            unsetenv("TEST_INHERITED_VARIABLE");
            REQUIRE(exec.success);
            REQUIRE(exec.exit_code == 0);
            auto variables = get_variables(exec.output);
            THEN("the child environment should contain the given variables") {
                REQUIRE(variables.size() > 4);
                REQUIRE(variables.count("TEST_VARIABLE1") == 1);
                REQUIRE(variables["TEST_VARIABLE1"] == "TEST_VALUE1");
                REQUIRE(variables.count("TEST_VARIABLE1") == 1);
                REQUIRE(variables["TEST_VARIABLE1"] == "TEST_VALUE1");
            }
            THEN("the child environment should have LC_ALL and LANG set to C") {
                REQUIRE(variables.count("LC_ALL") == 1);
                REQUIRE(variables["LC_ALL"] == "C");
                REQUIRE(variables.count("LANG") == 1);
                REQUIRE(variables["LANG"] == "C");
            }
        }
        WHEN("requested to override the environment") {
            setenv("TEST_INHERITED_VARIABLE", "TEST_INHERITED_VALUE", 1);
            auto exec = execute("env", {}, { {"TEST_VARIABLE1", "TEST_VALUE1" }, {"TEST_VARIABLE2", "TEST_VALUE2" }}, 0, { execution_options::trim_output });
            unsetenv("TEST_INHERITED_VARIABLE");
            REQUIRE(exec.success);
            REQUIRE(exec.exit_code == 0);
            auto variables = get_variables(exec.output);
            THEN("the child environment should only contain the given variables") {
                REQUIRE(variables.size() == 4u);
                REQUIRE(variables.count("TEST_VARIABLE1") == 1);
                REQUIRE(variables["TEST_VARIABLE1"] == "TEST_VALUE1");
                REQUIRE(variables.count("TEST_VARIABLE1") == 1);
                REQUIRE(variables["TEST_VARIABLE1"] == "TEST_VALUE1");
            }
            THEN("the child environment should have LC_ALL and LANG set to C") {
                REQUIRE(variables.count("LC_ALL") == 1);
                REQUIRE(variables["LC_ALL"] == "C");
                REQUIRE(variables.count("LANG") == 1);
                REQUIRE(variables["LANG"] == "C");
            }
        }
        WHEN("requested to override LC_ALL or LANG") {
            auto exec = execute("env", {}, { {"LANG", "FOO" }, { "LC_ALL", "BAR" } });
            REQUIRE(exec.success);
            REQUIRE(exec.exit_code == 0);
            auto variables = get_variables(exec.output);
            THEN("the values should be passed to the child process") {
                REQUIRE(variables.count("LC_ALL") == 1);
                REQUIRE(variables["LC_ALL"] == "BAR");
                REQUIRE(variables.count("LANG") == 1);
                REQUIRE(variables["LANG"] == "FOO");
            }
        }
    }
    GIVEN("a command that fails") {
        WHEN("default options are used") {
            auto exec = execute("ls", { "does_not_exist" });
            THEN("no output is returned") {
                REQUIRE_FALSE(exec.success);
                REQUIRE(exec.output == "");
                REQUIRE(exec.error == "");
                REQUIRE(exec.exit_code > 0);
            }
        }
        WHEN("the redirect stderr option is used") {
            auto exec = execute("ls", { "does_not_exist" }, 0, { execution_options::trim_output, execution_options::merge_environment, execution_options::redirect_stderr_to_stdout });
            THEN("error output is returned") {
                REQUIRE_FALSE(exec.success);
                REQUIRE(boost::ends_with(exec.output, "No such file or directory"));
                REQUIRE(exec.error == "");
                REQUIRE(exec.exit_code > 0);
            }
        }
        WHEN("not redirecting stderr to null") {
            auto exec = execute("ls", { "does_not_exist" }, 0, { execution_options::trim_output, execution_options::merge_environment });
            THEN("error output is returned") {
                REQUIRE_FALSE(exec.success);
                REQUIRE(exec.output == "");
                REQUIRE(boost::ends_with(exec.error, "No such file or directory"));
                REQUIRE(exec.exit_code > 0);
            }
        }
        WHEN("the 'throw on non-zero exit' option is used") {
            THEN("a child exit exception is thrown") {
                REQUIRE_THROWS_AS(execute("ls", {"does_not_exist"}, 0, {execution_options::trim_output, execution_options::merge_environment, execution_options::throw_on_nonzero_exit}), child_exit_exception);
            }
        }
        WHEN("the 'throw on signal' option is used") {
            THEN("a child signal exception is thrown") {
                REQUIRE_THROWS_AS(execute("sh", { EXEC_TESTS_DIRECTORY "/fixtures/execution/selfkill.sh" }, 0, { execution_options::trim_output, execution_options::merge_environment, execution_options::throw_on_signal }), child_signal_exception);
            }
        }
    }
    GIVEN("a command that outputs leading/trailing whitespace") {
        THEN("whitespace should be trimmed by default") {
            auto exec = execute("cat", { EXEC_TESTS_DIRECTORY "/fixtures/ls/file1.txt" });
            REQUIRE(exec.success);
            REQUIRE(exec.output == "this is a test of trimming");
        }
        WHEN("the 'trim whitespace' option is not used") {
            auto exec = execute("cat", { EXEC_TESTS_DIRECTORY "/fixtures/ls/file1.txt" }, 0, { execution_options::merge_environment });
            THEN("whitespace should not be trimmed") {
                REQUIRE(exec.output == "   this is a test of trimming   ");
            }
        }
    }
    GIVEN("a long-running command") {
        WHEN("given a timeout") {
            THEN("a timeout exception should be thrown") {
                try {
                    execute("sh", { EXEC_TESTS_DIRECTORY "/fixtures/execution/sleep.sh" }, 1);
                    FAIL("did not throw timeout exception");
                } catch (timeout_exception const& ex) {
                    // Verify the process group was killed by waiting for it
                    int status = 0;
                    REQUIRE(waitpid(-ex.pid(), &status, WNOHANG) == -1);
                    REQUIRE(errno == ECHILD);
                } catch (exception const&) {
                    FAIL("unexpected exception thrown");
                }
            }
        }
    }
    GIVEN("stderr is redirected to null") {
        WHEN("using a debug log level") {
            log_capture capture(log_level::debug);
            auto exec = execute(EXEC_TESTS_DIRECTORY "/fixtures/error_message");
            REQUIRE(exec.success);
            REQUIRE(exec.output == "foo=bar");
            REQUIRE(exec.error.empty());
            THEN("stderr is logged") {
                auto output = capture.result();
                CAPTURE(output);
                REQUIRE(re_search(output, boost::regex("DEBUG !!! - error message!")));
            }
        }
        WHEN("not using a debug log level") {
            log_capture capture(log_level::warning);
            auto exec = execute(EXEC_TESTS_DIRECTORY "/fixtures/error_message");
            REQUIRE(exec.success);
            REQUIRE(exec.output == "foo=bar");
            REQUIRE(exec.error.empty());
            THEN("stderr is not logged") {
                auto output = capture.result();
                CAPTURE(output);
                REQUIRE_FALSE(re_search(output, boost::regex("DEBUG !!! - error message!")));
            }
        }
    }
}

SCENARIO("executing commands with execution::each_line") {
    GIVEN("a command that succeeds") {
        THEN("each line of output should be returned") {
            vector<string> lines;
            bool success = each_line("cat", { EXEC_TESTS_DIRECTORY "/fixtures/ls/file4.txt" }, [&](string& line) {
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
            bool success = each_line("cat", { EXEC_TESTS_DIRECTORY "/fixtures/ls/file4.txt" }, [&](string& line) {
                lines.push_back(line);
                return false;
            });
            REQUIRE(success);
            REQUIRE(lines.size() == 1u);
            REQUIRE(lines[0] == "line1");
        }
        WHEN("requested to merge the environment") {
            setenv("TEST_INHERITED_VARIABLE", "TEST_INHERITED_VALUE", 1);
            map<string, string> variables;
            bool success = each_line("env", {}, { {"TEST_VARIABLE1", "TEST_VALUE1" }, {"TEST_VARIABLE2", "TEST_VALUE2" } }, [&](string& line) {
                vector<string> parts;
                boost::split(parts, line, boost::is_any_of("="), boost::token_compress_off);
                if (parts.size() != 2) {
                    return true;
                }
                variables.emplace(make_pair(move(parts[0]), move(parts[1])));
                return true;
            });
            unsetenv("TEST_INHERITED_VARIABLE");
            REQUIRE(success);
            THEN("the child environment should contain the given variables") {
                REQUIRE(variables.size() > 4);
                REQUIRE(variables.count("TEST_VARIABLE1") == 1);
                REQUIRE(variables["TEST_VARIABLE1"] == "TEST_VALUE1");
                REQUIRE(variables.count("TEST_VARIABLE1") == 1);
                REQUIRE(variables["TEST_VARIABLE1"] == "TEST_VALUE1");
            }
            THEN("the child environment should have LC_ALL and LANG set to C") {
                REQUIRE(variables.count("LC_ALL") == 1);
                REQUIRE(variables["LC_ALL"] == "C");
                REQUIRE(variables.count("LANG") == 1);
                REQUIRE(variables["LANG"] == "C");
            }
        }
        WHEN("requested to override the environment") {
            setenv("TEST_INHERITED_VARIABLE", "TEST_INHERITED_VALUE", 1);
            map<string, string> variables;
            bool success = each_line(
                "env",
                {},
                {
                    {"TEST_VARIABLE1", "TEST_VALUE1" },
                    {"TEST_VARIABLE2", "TEST_VALUE2" }
                },
                [&](string& line) {
                    vector<string> parts;
                    boost::split(parts, line, boost::is_any_of("="), boost::token_compress_off);
                    if (parts.size() != 2) {
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
            unsetenv("TEST_INHERITED_VARIABLE");
            REQUIRE(success);
            THEN("the child environment should only contain the given variables") {
                REQUIRE(variables.size() == 4u);
                REQUIRE(variables.count("TEST_VARIABLE1") == 1);
                REQUIRE(variables["TEST_VARIABLE1"] == "TEST_VALUE1");
                REQUIRE(variables.count("TEST_VARIABLE1") == 1);
                REQUIRE(variables["TEST_VARIABLE1"] == "TEST_VALUE1");
            }
            THEN("the child environment should have LC_ALL and LANG set to C") {
                REQUIRE(variables.count("LC_ALL") == 1);
                REQUIRE(variables["LC_ALL"] == "C");
                REQUIRE(variables.count("LANG") == 1);
                REQUIRE(variables["LANG"] == "C");
            }
        }
        WHEN("requested to override LC_ALL or LANG") {
            map<string, string> variables;
            bool success = each_line("env", {}, { {"LANG", "FOO" }, { "LC_ALL", "BAR" } }, [&](string& line) {
                vector<string> parts;
                boost::split(parts, line, boost::is_any_of("="), boost::token_compress_off);
                if (parts.size() != 2) {
                    return true;
                }
                variables.emplace(make_pair(move(parts[0]), move(parts[1])));
                return true;
            });
            REQUIRE(success);
            THEN("the values should be passed to the child process") {
                REQUIRE(variables.count("LC_ALL") == 1);
                REQUIRE(variables["LC_ALL"] == "BAR");
                REQUIRE(variables.count("LANG") == 1);
                REQUIRE(variables["LANG"] == "FOO");
            }
        }
    }
    GIVEN("a command that fails") {
        WHEN("default options are used") {
            THEN("no output is returned") {
                auto success = each_line("ls", { "does_not_exist" }, [](string& line) {
                    FAIL("should not be called");
                    return true;
                });
                REQUIRE_FALSE(success);
            }
        }
        WHEN("the redirect stderr option is used") {
            string output;
            auto result = each_line(
                "ls",
                {
                    "does_not_exist"
                },
                [&](string& line) {
                    if (!output.empty()) {
                        output += "\n";
                    }
                    output += line;
                    return true;
                },
                nullptr,
                0,
                {
                    execution_options::trim_output,
                    execution_options::merge_environment,
                    execution_options::redirect_stderr_to_stdout
                });
            THEN("error output is returned") {
                REQUIRE_FALSE(result);
                REQUIRE(boost::ends_with(output, "No such file or directory"));
            }
        }
        WHEN("not redirecting stderr to null") {
            string output;
            string error;
            auto result = each_line(
                "ls",
                {
                    "does_not_exist"
                },
                [&](string& line) {
                    if (!output.empty()) {
                        output += "\n";
                    }
                    output += line;
                    return true;
                },
                [&](string& line) {
                    if (!error.empty()) {
                        error += "\n";
                    }
                    error += line;
                    return true;
                });
            THEN("error output is returned") {
                REQUIRE_FALSE(result);
                REQUIRE(output == "");
                REQUIRE(boost::ends_with(error, "No such file or directory"));
            }
        }
        WHEN("redirecting stderr to null") {
            string error;
            auto result = each_line(
                "ls",
                {
                    "does_not_exist"
                },
                nullptr,
                [&](string& line) {
                    if (!error.empty()) {
                        error += "\n";
                    }
                    error += line;
                    return true;
                },
                0,
                {
                    execution_options::trim_output,
                    execution_options::merge_environment,
                    execution_options::redirect_stderr_to_null
                });
            THEN("no error output is returned") {
                REQUIRE_FALSE(result);
                REQUIRE(error == "");
            }
        }
        WHEN("the 'throw on non-zero exit' option is used") {
            THEN("a child exit exception is thrown") {
                REQUIRE_THROWS_AS(each_line("ls", {"does_not_exist"}, nullptr, nullptr, 0, {execution_options::trim_output, execution_options::merge_environment, execution_options::throw_on_nonzero_exit}), child_exit_exception);
            }
        }
        WHEN("the 'throw on signal' option is used") {
            THEN("a child signal exception is thrown") {
                REQUIRE_THROWS_AS(each_line("sh", { EXEC_TESTS_DIRECTORY "/fixtures/execution/selfkill.sh" }, nullptr, nullptr, 0, { execution_options::trim_output, execution_options::merge_environment, execution_options::throw_on_signal }), child_signal_exception);
            }
        }
    }
    GIVEN("a long-running command") {
        WHEN("given a timeout") {
            THEN("a timeout exception should be thrown") {
                try {
                    each_line("sh", { EXEC_TESTS_DIRECTORY "/fixtures/execution/sleep.sh" }, nullptr, nullptr, 1);
                    FAIL("did not throw timeout exception");
                } catch (timeout_exception const& ex) {
                    // Verify the process group was killed by waiting for it
                    int status = 0;
                    REQUIRE(waitpid(-ex.pid(), &status, WNOHANG) == -1);
                    REQUIRE(errno == ECHILD);
                } catch (exception const&) {
                    FAIL("unexpected exception thrown");
                }
            }
        }
    }
    GIVEN("stderr is redirected to null") {
        WHEN("using a debug log level") {
            log_capture capture(log_level::debug);
            REQUIRE(leatherman::execution::each_line(EXEC_TESTS_DIRECTORY "/fixtures/error_message", nullptr));
            THEN("stderr is logged") {
                auto output = capture.result();
                CAPTURE(output);
                REQUIRE(re_search(output, boost::regex("DEBUG !!! - error message!")));
            }
        }
        WHEN("not using a debug log level") {
            log_capture capture(log_level::warning);
            REQUIRE(leatherman::execution::each_line(EXEC_TESTS_DIRECTORY "/fixtures/error_message", nullptr));
            THEN("stderr is not logged") {
                auto output = capture.result();
                CAPTURE(output);
                REQUIRE_FALSE(re_search(output, boost::regex("DEBUG !!! - error message!")));
            }
        }
    }
}

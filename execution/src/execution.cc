#include <leatherman/execution/execution.hpp>
#include <leatherman/logging/logging.hpp>
#include <leatherman/locale/locale.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/filesystem.hpp>
#include <boost/optional.hpp>
#include <boost/nowide/fstream.hpp>
#include <cstdlib>
#include <cstdio>
#include <sstream>
#include <cstring>

// Mark string for translation (alias for leatherman::locale::format)
using leatherman::locale::_;

using namespace std;
using namespace leatherman::logging;
using namespace leatherman::util;
namespace fs = boost::filesystem;
using namespace boost::algorithm;

namespace leatherman { namespace execution {

    execution_exception::execution_exception(string const& message) :
        runtime_error(message)
    {
    }

    execution_failure_exception::execution_failure_exception(string const& message, string output, string error) :
        execution_exception(message),
        _output(move(output)),
        _error(move(error))
    {
    }

    string const& execution_failure_exception::output() const
    {
        return _output;
    }

    string const& execution_failure_exception::error() const
    {
        return _error;
    }

    child_exit_exception::child_exit_exception(string const& message, int status_code, string output, string error) :
        execution_failure_exception(message, move(output), move(error)),
        _status_code(status_code)
    {
    }

    int child_exit_exception::status_code() const
    {
        return _status_code;
    }

    child_signal_exception::child_signal_exception(string const& message, int signal, string output, string error) :
        execution_failure_exception(message, move(output), move(error)),
        _signal(signal)
    {
    }

    int child_signal_exception::signal() const
    {
        return _signal;
    }

    timeout_exception::timeout_exception(string const& message, size_t pid) :
        execution_exception(message),
        _pid(pid)
    {
    }

    size_t timeout_exception::pid() const
    {
        return _pid;
    }

    void log_execution(string const& file, vector<string> const* arguments)
    {
        if (!LOG_IS_DEBUG_ENABLED()) {
            return;
        }

        ostringstream command_line;
        command_line << file;

        if (arguments) {
            for (auto const& argument : *arguments) {
                command_line << ' ' << argument;
            }
        }
        LOG_DEBUG("executing command: {1}", command_line.str());
    }

    string expand_command(string const& command, vector<string> const& directories)
    {
        string result = command;
        boost::trim(result);

        if (result.empty()) {
            return {};
        }

        bool quoted = result[0] == '"' || result[0] == '\'';

        string file;
        string remainder;
        if (quoted) {
            // Look for the ending quote for the command
            auto pos = result.find(result[0], 1);
            if (pos == string::npos) {
                // No closing quote
                file = result.substr(1);
            } else {
                file = result.substr(1, pos - 1);
                remainder = result.substr(pos + 1);
            }
        } else {
            auto pos = command.find(' ');
            if (pos == string::npos) {
                file = result;
            } else {
                file = result.substr(0, pos);
                remainder = result.substr(pos);
            }
        }

        file = which(file, directories);
        if (file.empty()) {
            return {};
        }

        // If originally unquoted and the expanded file has a space, quote it
        if (!quoted && file.find(' ') != string::npos) {
            return "\"" + file + "\"" + remainder;
        } else if (quoted) {
            // Originally quoted, use the same quote characters
            return result[0] + file + result[0] + remainder;
        }
        // Not quoted
        return file + remainder;
    }

    result execute(
        string const& file,
        vector<string> const* arguments,
        string const* input,
        map<string, string> const* environment,
        function<void(size_t)> const& pid_callback,
        function<bool(string&)> const& stdout_callback,
        function<bool(string&)> const& stderr_callback,
        option_set<execution_options> const& options,
        uint32_t timeout);

    static void setup_execute(function<bool(string&)>& stderr_callback, option_set<execution_options>& options)
    {
        // If not redirecting stderr to stdout, but redirecting to null, use a do-nothing callback so that stderr is logged when the level is debug
        if (LOG_IS_DEBUG_ENABLED() && !options[execution_options::redirect_stderr_to_stdout] && options[execution_options::redirect_stderr_to_null]) {
            // Use a do-nothing callback so that stderr is logged
            stderr_callback = ([&](string&) {
                return true;
            });
            options.clear(execution_options::redirect_stderr_to_null);
        }
    }

    result execute(
        string const& file,
        uint32_t timeout,
        option_set<execution_options> const& options)
    {
        auto actual_options = options;
        function<bool(string&)> stderr_callback;
        setup_execute(stderr_callback, actual_options);
        return execute(file, nullptr, nullptr, nullptr, nullptr, nullptr, stderr_callback, actual_options, timeout);
    }

    result execute(
        string const& file,
        vector<string> const& arguments,
        uint32_t timeout,
        option_set<execution_options> const& options)
    {
        auto actual_options = options;
        function<bool(string&)> stderr_callback;
        setup_execute(stderr_callback, actual_options);
        return execute(file, &arguments, nullptr, nullptr, nullptr, nullptr, stderr_callback, actual_options, timeout);
    }

    result execute(
        string const& file,
        vector<string> const& arguments,
        map<string, string> const& environment,
        uint32_t timeout,
        option_set<execution_options> const& options)
    {
        auto actual_options = options;
        function<bool(string&)> stderr_callback;
        setup_execute(stderr_callback, actual_options);
        return execute(file, &arguments, nullptr, &environment, nullptr, nullptr, stderr_callback, actual_options, timeout);
    }

    result execute(
        string const& file,
        vector<string> const& arguments,
        string const& input,
        uint32_t timeout,
        option_set<execution_options> const& options)
    {
        auto actual_options = options;
        function<bool(string&)> stderr_callback;
        setup_execute(stderr_callback, actual_options);
        return execute(file, &arguments, &input, nullptr, nullptr, nullptr, stderr_callback, actual_options, timeout);
    }

    result execute(
        string const& file,
        vector<string> const& arguments,
        string const& input,
        map<string, string> const& environment,
        uint32_t timeout,
        option_set<execution_options> const& options)
    {
        auto actual_options = options;
        function<bool(string&)> stderr_callback;
        setup_execute(stderr_callback, actual_options);
        return execute(file, &arguments, &input, &environment, nullptr, nullptr, stderr_callback, actual_options, timeout);
    }

    result execute(
        string const& file,
        vector<string> const& arguments,
        string const& input,
        map<string, string> const& environment,
        // cppcheck-suppress passedByValue
        std::function<void(size_t)> pid_callback,
        uint32_t timeout,
        option_set<execution_options> const& options)
    {
        auto actual_options = options;
        function<bool(string&)> stderr_callback;
        setup_execute(stderr_callback, actual_options);
        return execute(file, &arguments, &input, &environment, pid_callback, nullptr, stderr_callback, actual_options, timeout);
    }

    result execute(
        std::string const& file,
        std::vector<std::string> const& arguments,
        std::string const& input,
        std::string const& out_file,
        std::string const& err_file,
        std::map<std::string, std::string> const& environment,
        std::function<void(size_t)> pid_callback,
        uint32_t timeout,
        lth_util::option_set<execution_options> const& options)
    {
        return execute(file, arguments, input, out_file, err_file, environment, pid_callback, timeout, {}, options);
    }

    result execute(
        // cppcheck-suppress funcArgOrderDifferent
        std::string const& file,
        std::vector<std::string> const& arguments,
        std::string const& input,
        std::string const& out_file,
        std::string const& err_file,
        std::map<std::string, std::string> const& environment,
        // cppcheck-suppress passedByValue
        std::function<void(size_t)> pid_callback,
        uint32_t timeout,
        boost::optional<fs::perms> perms,
        lth_util::option_set<execution_options> const& options)
    {
        auto actual_options = options;
        function<bool(string&)> stderr_callback;
        function<bool(string&)> stdout_callback;
        boost::nowide::ofstream out_stream;
        boost::nowide::ofstream err_stream;

        out_stream.open(out_file.c_str(), std::ios::binary);
        if (!out_stream.is_open()) {
            throw execution_exception(_("failed to open output file {1}", out_file));
        }

        boost::system::error_code ec;
        if (perms) {
            fs::permissions(out_file, *perms, ec);
            if (ec) {
                throw execution_exception(_("failed to modify permissions on output file {1} to {2,num,oct}: {3}", out_file, *perms, ec.message()));
            }
        }

        if (err_file.empty()) {
            setup_execute(stderr_callback, actual_options);
        } else {
            err_stream.open(err_file.c_str(), std::ios::binary);
            if (!err_stream.is_open()) {
                throw execution_exception(_("failed to open error file {1}", err_file));
            }

            if (perms) {
                fs::permissions(err_file, *perms, ec);
                if (ec) {
                    throw execution_exception(_("failed to modify permissions on error file {1} to {2,num,oct}: {3}", err_file, *perms, ec.message()));
                }
            }

            stderr_callback = ([&](string& line) {
                err_stream << line << "\n";
                return true;
            });
        }

        stdout_callback = ([&](string& line) {
            out_stream << line << "\n";
            return true;
        });

        auto env = environment.empty() ? nullptr : &environment;

        return execute(file, &arguments, &input, env, pid_callback, stdout_callback, stderr_callback, actual_options, timeout);
    }

    static void setup_each_line(function<bool(string&)>& stdout_callback, function<bool(string&)>& stderr_callback, option_set<execution_options>& options)
    {
        // If not given a stdout callback, use a no-op one to prevent execute from buffering stdout (also logs for debug level)
        if (!stdout_callback) {
            stdout_callback = ([&](string&) {
                return true;
            });
        }
        // If given no stderr callback and not redirecting to stdout, redirect stderr to null when not debug log level
        if (!stderr_callback && !options[execution_options::redirect_stderr_to_stdout]) {
            if (LOG_IS_DEBUG_ENABLED()) {
                // Use a do-nothing callback so that stderr is logged
                stderr_callback = ([&](string&) {
                    return true;
                });
                options.clear(execution_options::redirect_stderr_to_null);
            } else {
                // Not debug level, redirect to null
                options.set(execution_options::redirect_stderr_to_null);
            }
        }
    }

    bool each_line(
        string const& file,
        function<bool(string&)> stdout_callback,
        function<bool(string&)> stderr_callback,
        uint32_t timeout,
        option_set<execution_options> const& options)
    {
        auto actual_options = options;
        setup_each_line(stdout_callback, stderr_callback, actual_options);
        return execute(file, nullptr, nullptr, nullptr, nullptr, stdout_callback, stderr_callback, actual_options, timeout).success;
    }

    bool each_line(
        string const& file,
        vector<string> const& arguments,
        function<bool(string&)> stdout_callback,
        function<bool(string&)> stderr_callback,
        uint32_t timeout,
        option_set<execution_options> const& options)
    {
        auto actual_options = options;
        setup_each_line(stdout_callback, stderr_callback, actual_options);
        return execute(file, &arguments, nullptr, nullptr, nullptr, stdout_callback, stderr_callback, actual_options, timeout).success;
    }

    bool each_line(
        string const& file,
        vector<string> const& arguments,
        map<string, string> const& environment,
        function<bool(string&)> stdout_callback,
        function<bool(string&)> stderr_callback,
        uint32_t timeout,
        option_set<execution_options> const& options)
    {
        auto actual_options = options;
        setup_each_line(stdout_callback, stderr_callback, actual_options);
        return execute(file, &arguments, nullptr, &environment, nullptr, stdout_callback, stderr_callback, actual_options, timeout).success;
    }

    static bool process_data(bool trim, string const& data, string& buffer, string const& logger, function<bool(string&)> const& callback)
    {
        // Do nothing if nothing was read
        if (data.empty()) {
            return true;
        }

        // If given no callback, buffer the entire output
        if (!callback) {
            buffer.append(data);
            return true;
        }

        // Find the last newline, because anything after may not be a complete line.
        auto lastNL = data.find_last_of("\n");
        if (lastNL == string::npos) {
            // No newline found, so keep appending and continue.
            buffer.append(data);
            return true;
        }

        // Make a range for iterating through lines.
        auto str_range = make_pair(data.begin(), data.begin()+lastNL);
        auto line_iterator = boost::make_iterator_range(
                make_split_iterator(str_range, token_finder(is_any_of("\n"))),
                split_iterator<string::const_iterator>());

        for (auto &line : line_iterator) {
            // The previous trailing data is picked up by default.
            buffer.append(line.begin(), line.end());

            if (trim) {
                boost::trim(buffer);

                // Skip empty lines only if trimming output.
                // Otherwise we want to include empty lines to remain honest to the original output.
                if (buffer.empty()) {
                    continue;
                }
            }

#ifdef _WIN32
            // Remove leading or trailing carriage returns. We don't want them during callbacks.
            boost::trim_if(buffer, is_any_of("\r"));
#endif

            // Log the line to the output logger
            if (LOG_IS_DEBUG_ENABLED()) {
                log(logger, log_level::debug, 0, buffer);
            }

            // Pass the line to the callback
            bool finished = !callback(buffer);

            // Clear the line for the next iteration
            buffer.clear();

            // Break out if finished processing
            if (finished) {
                return false;
            }
        }

        // Save the new trailing data; step past the newline
        buffer.assign(data.begin()+lastNL+1, data.end());
        return true;
    }

    tuple<string, string> process_streams(bool trim, function<bool(string&)> const& stdout_callback, function<bool(string&)> const& stderr_callback, function<void(function<bool(string const&)>, function<bool(string const&)>)> const& read_streams)
    {
        // Get a special logger used specifically for child process output
        static const string stdout_logger = "|";
        static const string stderr_logger = "!!!";

        // Buffers for all of the output or partial line output
        string stdout_buffer;
        string stderr_buffer;

        // Read the streams
        read_streams(
            [&](string const& data) {
                if (!process_data(trim, data, stdout_buffer, stdout_logger, stdout_callback)) {
                    LOG_DEBUG("completed processing output: closing child pipes.");
                    return false;
                }
                return true;
            },
            [&](string const& data) {
                if (!process_data(trim, data, stderr_buffer, stderr_logger, stderr_callback)) {
                    LOG_DEBUG("completed processing output: closing child pipes.");
                    return false;
                }
                return true;
            });

        // Log the result and do a final callback if needed.
        if (trim) {
            boost::trim(stdout_buffer);
            boost::trim(stderr_buffer);
        }
        // Log the last line of output for stdout
        if (!stdout_buffer.empty()) {
            if (LOG_IS_DEBUG_ENABLED()) {
                log(stdout_logger, log_level::debug, 0, stdout_buffer);
            }
            if (stdout_callback) {
                stdout_callback(stdout_buffer);
                stdout_buffer.clear();
            }
        }
        // Log the last line of output for stderr
        if (!stderr_buffer.empty()) {
            if (LOG_IS_DEBUG_ENABLED()) {
                log(stderr_logger, log_level::debug, 0, stderr_buffer);
            }
            if (stderr_callback) {
                stderr_callback(stderr_buffer);
                stderr_buffer.clear();
            }
        }
        return make_tuple(move(stdout_buffer), move(stderr_buffer));
    }

}}  // namespace leatherman::execution

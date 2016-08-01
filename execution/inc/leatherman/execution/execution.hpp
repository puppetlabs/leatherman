/**
 * @file
 * Declares functions used for executing commands.
 */
#pragma once

#include <leatherman/util/environment.hpp>
#include <leatherman/util/option_set.hpp>

#include <string>
#include <vector>
#include <map>
#include <tuple>
#include <utility>
#include <stdexcept>
#include <functional>

namespace lth_util = leatherman::util;

namespace leatherman { namespace execution {

    /**
     * The supported execution options.
     */
    enum class execution_options
    {
        /**
         * No options.
         */
        none = 0,
        /**
         * Redirect stderr to stdout.  This will override redirect_stderr_to_null if both are set.
         */
        redirect_stderr_to_stdout = (1 << 1),
        /**
         * Throw an exception if the child process exits with a nonzero status.
         */
        throw_on_nonzero_exit = (1 << 2),
        /**
         * Throw an exception if the child process is terminated due to a signal.
         */
        throw_on_signal = (1 << 3),
        /**
         * Automatically trim output leading and trailing whitespace.
         */
        trim_output = (1 << 4),
        /**
         * Merge specified environment with the current process environment.
         */
        merge_environment = (1 << 5),
        /**
         * Redirect stderr to "null".
         */
        redirect_stderr_to_null = (1 << 6),
        /**
         * Preserve (do not quote) arguments.
         */
        preserve_arguments = (1 << 7),
        /**
         * On Windows, create a new process group for the child process, but not a Job Object.
         */
        create_new_process_group = (1 << 8),
        /**
         * Inherit locale environment variables from the current process. Limited to LC_ALL and
         * LOCALE, which are specifically overridden to "C" with merge_environment.
         * Will not override those variables if explicitly passed in an environment map.
         */
        inherit_locale = (1 << 9),
        /**
         * A combination of all throw options.
         */
        throw_on_failure = throw_on_nonzero_exit | throw_on_signal,
    };

    /**
     * System command shell available for executing shell scripts.
     * Uses 'cmd' on Windows and 'sh' on *nix systems.
     */
    extern const char *const command_shell;

    /**
     * System command shell arguments to accept a script as an argument.
     * Uses '/c' on Windows and '-c' on *nix systems.
     */
    extern const char *const command_args;

    /**
     * Base class for execution exceptions.
     */
    struct execution_exception : std::runtime_error
    {
        /**
         * Constructs a execution_exception.
         * @param message The exception message.
         */
        explicit execution_exception(std::string const& message);
    };

    /**
     * Base class for execution failures.
     */
    struct execution_failure_exception : execution_exception
    {
        /**
         * Constructs a execution_failure_exception.
         * @param message The exception message.
         * @param output The child process stdout output.
         * @param error The child process stderr output.
         */
        execution_failure_exception(std::string const& message, std::string output, std::string error);

        /**
         * Gets the child process stdout output.
         * @return Returns the child process stdout output.
         */
        std::string const& output() const;

        /**
         * Gets the child process stderr output.
         * @return Returns the child process stderr output.
         */
        std::string const& error() const;

     private:
        std::string _output;
        std::string _error;
    };

    /**
     * Exception that is thrown when a child exits with a non-zero status code.
     */
    struct child_exit_exception : execution_failure_exception
    {
        /**
         * Constructs a child_exit_exception.
         * @param message The exception message.
         * @param status_code The exit status code of the child process.
         * @param output The child process stdout output.
         * @param error The child process stderr output.
         */
        child_exit_exception(std::string const& message, int status_code, std::string output, std::string error);

        /**
         * Gets the child process exit status code.
         * @return Returns the child process exit status code.
         */
        int status_code() const;

     private:
        int _status_code;
    };

    /**
     * Exception that is thrown when a child exists due to a signal.
     */
    struct child_signal_exception : execution_failure_exception
    {
        /**
         * Constructs a child_signal_exception.
         * @param message The exception message.
         * @param signal The signal code that terminated the child process.
         * @param output The child process stdout output.
         * @param error The child process stderr output.
         */
        child_signal_exception(std::string const& message, int signal, std::string output, std::string error);

        /**
         * Gets the signal that terminated the child process.
         * @return Returns the signal that terminated the child process.
         */
        int signal() const;

     private:
        int _signal;
    };

    /**
     * Exception that is thrown when a command times out.
     */
    struct timeout_exception : execution_exception
    {
        /**
         * Constructs a timeout_exception.
         * @param message The exception message.
         * @param pid The process id of the process that timed out and was killed.
         */
        timeout_exception(std::string const& message, size_t pid);

        /**
         * Gets the process id of the process that timed out and was killed.
         * @return Returns the process id of the process that timed out and was killed.
         */
        size_t pid() const;

     private:
        size_t _pid;
    };

    /**
     * Encapsulates return value from executing a process.
     */
    struct result
    {
        /**
         * Constructor.
         */
        result(bool s, std::string o, std::string e, int ec, size_t p)
            : success(s), output(move(o)), error(move(e)), exit_code(ec), pid(p) {}
        /**
         * Whether or not the command succeeded, defaults to true.
         */
        bool success = true;
        /**
         * Output from stdout.
         */
        std::string output;
        /**
         * Output from stderr (if not redirected).
         */
        std::string error;
        /**
         * The process exit code, defaults to 0.
         */
        int exit_code = 0;
        /**
         * The process ID
         */
        size_t pid = 0;
    };

    /**
     * Searches the given paths for the given executable file.
     * @param file The file to search for.
     * @param directories The directories to search.
     * @return Returns the full path or empty if the file could not be found.
     */
    std::string which(std::string const& file, std::vector<std::string> const& directories = lth_util::environment::search_paths());

    /**
     * Expands the executable in the command to the full path.
     * @param command The command to expand.
     * @param directories The directories to search.
     * @return Returns the expanded command if the executable was found or empty if it was not found..
     */
    std::string expand_command(std::string const& command, std::vector<std::string> const& directories = lth_util::environment::search_paths());

    /**
     * Executes the given program.
     * @param file The name or path of the program to execute.
     * @param timeout The timeout, in seconds.  Defaults to no timeout.
     * @param options The execution options.  Defaults to trimming output, merging the environment, and redirecting stderr to null.
     * @return Returns a result struct.
     */
    result execute(
        std::string const& file,
        uint32_t timeout = 0,
        lth_util::option_set<execution_options> const& options = { execution_options::trim_output, execution_options::merge_environment, execution_options::redirect_stderr_to_null });

    /**
     * Executes the given program.
     * @param file The name or path of the program to execute.
     * @param arguments The arguments to pass to the program. On Windows they will be quoted as needed for spaces.
     * @param timeout The timeout, in seconds. Defaults to no timeout.
     * @param options The execution options.  Defaults to trimming output, merging the environment, and redirecting stderr to null.
     * @return Returns a result struct.
     */
    result execute(
        std::string const& file,
        std::vector<std::string> const& arguments,
        uint32_t timeout = 0,
        lth_util::option_set<execution_options> const& options = { execution_options::trim_output, execution_options::merge_environment, execution_options::redirect_stderr_to_null });

    /**
     * Executes the given program.
     * @param file The name or path of the program to execute.
     * @param arguments The arguments to pass to the program. On Windows they will be quoted as needed for spaces.
     * @param environment The environment variables to pass to the child process.
     * @param timeout The timeout, in seconds. Defaults to no timeout.
     * @param options The execution options.  Defaults to trimming output, merging the environment, and redirecting stderr to null.
     * @return Returns a result struct.
     */
    result execute(
        std::string const& file,
        std::vector<std::string> const& arguments,
        std::map<std::string, std::string> const& environment,
        uint32_t timeout = 0,
        lth_util::option_set<execution_options> const& options = { execution_options::trim_output, execution_options::merge_environment, execution_options::redirect_stderr_to_null });

    /**
     * Executes the given program.
     * @param file The name or path of the program to execute.
     * @param arguments The arguments to pass to the program. On Windows they will be quoted as needed for spaces.
     * @param input A string to place on stdin for the child process before reading output.
     * @param timeout The timeout, in seconds. Defaults to no timeout.
     * @param options The execution options.  Defaults to trimming output, merging the environment, and redirecting stderr to null.
     * @return Returns a result struct.
     */
    result execute(
        std::string const& file,
        std::vector<std::string> const& arguments,
        std::string const& input,
        uint32_t timeout = 0,
        lth_util::option_set<execution_options> const& options = { execution_options::trim_output, execution_options::merge_environment, execution_options::redirect_stderr_to_null });

    /**
     * Executes the given program.
     * @param file The name or path of the program to execute.
     * @param arguments The arguments to pass to the program. On Windows they will be quoted as needed for spaces.
     * @param input A string to place on stdin for the child process before reading output.
     * @param environment The environment variables to pass to the child process.
     * @param timeout The timeout, in seconds. Defaults to no timeout.
     * @param options The execution options.  Defaults to trimming output, merging the environment, and redirecting stderr to null.
     * @return Returns a result struct.
     */
    result execute(
        std::string const& file,
        std::vector<std::string> const& arguments,
        std::string const& input,
        std::map<std::string, std::string> const& environment,
        uint32_t timeout = 0,
        lth_util::option_set<execution_options> const& options = { execution_options::trim_output, execution_options::merge_environment, execution_options::redirect_stderr_to_null });

    /**
     * Executes the given program by calling a specified callback that receives the pid of the program's process.
     * @param file The name or path of the program to execute.
     * @param arguments The arguments to pass to the program. On Windows they will be quoted as needed for spaces.
     * @param input A string to place on stdin for the child process before reading output.
     * @param environment The environment variables to pass to the child process.
     * @param pid_callback The callback that is called with the pid of the child process. Defaults to no callback, in which case the pid won't be processed.
     * @param timeout The timeout, in seconds. Defaults to no timeout.
     * @param options The execution options.  Defaults to trimming output, merging the environment, and redirecting stderr to null.
     * @return Returns a result struct.
     */
    result execute(
        std::string const& file,
        std::vector<std::string> const& arguments,
        std::string const& input,
        std::map<std::string, std::string> const& environment,
        std::function<void(size_t)> pid_callback = nullptr,
        uint32_t timeout = 0,
        lth_util::option_set<execution_options> const& options = { execution_options::trim_output, execution_options::merge_environment, execution_options::redirect_stderr_to_null });

    /**
     * Executes the given program by writing the output of stdout and stderr to specified files.
     * @param file The name or path of the program to execute.
     * @param arguments The arguments to pass to the program. On Windows they will be quoted as needed for spaces.
     * @param input A string to place on stdin for the child process before reading output.
     * @param out_file The file where the output on stdout will be written.
     * @param err_file The file where the output on stderr will be written. Defaults to no file, in which case the output on stderr will be buffered and returned in the result struct.
     * @param environment The environment variables to pass to the child process.
     * @param pid_callback The callback that is called with the pid of the child process. Defaults to no callback, in which case the pid will not be processed.
     * @param timeout The timeout, in seconds. Defaults to no timeout.
     * @param options The execution options.  Defaults to trimming output and merging the environment.
     * @return Returns a result struct that will not contain the output of the streams for which a file was specified.
     *
     * Throws an execution_exception error in case it fails to open a file.
     */
    result execute(
        std::string const& file,
        std::vector<std::string> const& arguments,
        std::string const& input,
        std::string const& out_file,
        std::string const& err_file = "",
        std::map<std::string, std::string> const& environment = std::map<std::string, std::string>(),
        std::function<void(size_t)> pid_callback = nullptr,
        uint32_t timeout = 0,
        lth_util::option_set<execution_options> const& options = { execution_options::trim_output, execution_options::merge_environment });

    /**
     * Executes the given program and returns each line of output.
     * @param file The name or path of the program to execute.
     * @param stdout_callback The callback that is called with each line of output on stdout.
     * @param stderr_callback The callback that is called with each line of output on stderr. If nullptr, implies redirect_stderr_to_null unless redirect_stderr_to_stdout is set in options.
     * @param timeout The timeout, in seconds. Defaults to no timeout.
     * @param options The execution options.  Defaults to trimming output and merging the environment.
     * @return Returns true if the execution succeeded or false if it did not.
     */
    bool each_line(
        std::string const& file,
        std::function<bool(std::string&)> stdout_callback,
        std::function<bool(std::string&)> stderr_callback = nullptr,
        uint32_t timeout = 0,
        lth_util::option_set<execution_options> const& options = { execution_options::trim_output, execution_options::merge_environment });

    /**
     * Executes the given program and returns each line of output.
     * @param file The name or path of the program to execute.
     * @param arguments The arguments to pass to the program. On Windows they will be quoted as needed for spaces.
     * @param stdout_callback The callback that is called with each line of output on stdout.
     * @param stderr_callback The callback that is called with each line of output on stderr. If nullptr, implies redirect_stderr_to_null unless redirect_stderr_to_stdout is set in options.
     * @param timeout The timeout, in seconds. Defaults to no timeout.
     * @param options The execution options.  Defaults to trimming output and merging the environment.
     * @return Returns true if the execution succeeded or false if it did not.
     */
    bool each_line(
        std::string const& file,
        std::vector<std::string> const& arguments,
        std::function<bool(std::string&)> stdout_callback,
        std::function<bool(std::string&)> stderr_callback = nullptr,
        uint32_t timeout = 0,
        lth_util::option_set<execution_options> const& options = { execution_options::trim_output, execution_options::merge_environment });

    /**
     * Executes the given program and returns each line of output.
     * @param file The name or path of the program to execute.
     * @param arguments The arguments to pass to the program. On Windows they will be quoted as needed for spaces.
     * @param environment The environment variables to pass to the child process.
     * @param stdout_callback The callback that is called with each line of output on stdout.
     * @param stderr_callback The callback that is called with each line of output on stderr. If nullptr, implies redirect_stderr_to_null unless redirect_to_stdout is set in options.
     * @param timeout The timeout, in seconds. Defaults to no timeout.
     * @param options The execution options.  Defaults to trimming output and merging the environment.
     * @return Returns true if the execution succeeded or false if it did not.
     */
    bool each_line(
        std::string const& file,
        std::vector<std::string> const& arguments,
        std::map<std::string, std::string> const& environment,
        std::function<bool(std::string&)> stdout_callback,
        std::function<bool(std::string&)> stderr_callback = nullptr,
        uint32_t timeout = 0,
        lth_util::option_set<execution_options> const& options = { execution_options::trim_output, execution_options::merge_environment });

    /**
     * Processes stdout and stderror streams of a child process.
     * @param trim True if output should be trimmed or false if not.
     * @param stdout_callback The callback to use when a line is read for stdout.
     * @param stderr_callback The callback to use when a line is read for stderr.
     * @param read_streams The callback that is called to read stdout and stderr streams.
     * @return Returns a tuple of stdout and stderr output.  If stdout_callback or stderr_callback is given, it will return empty strings.
     */
    std::tuple<std::string, std::string> process_streams(
            bool trim,
            std::function<bool(std::string&)> const& stdout_callback,
            std::function<bool(std::string&)> const& stderr_callback,
            std::function<void(std::function<bool(std::string const&)>, std::function<bool(std::string const&)>)> const& read_streams);

}}  // namespace leatherman::execution

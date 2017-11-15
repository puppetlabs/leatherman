#include <leatherman/execution/execution.hpp>
#include <leatherman/file_util/directory.hpp>
#include <leatherman/util/scope_exit.hpp>
#include <leatherman/util/posix/scoped_descriptor.hpp>
#include <leatherman/logging/logging.hpp>
#include <leatherman/locale/locale.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/filesystem.hpp>
#include <array>

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <fcntl.h>
#include <signal.h>

#include "platform.hpp"

// Mark string for translation (alias for leatherman::locale::format)
using leatherman::locale::_;

using namespace std;
using namespace leatherman::util;
using namespace leatherman::util::posix;
using namespace leatherman::execution;
using namespace leatherman::logging;
using namespace leatherman::file_util;
using namespace boost::filesystem;

// Declare environ for OSX
extern char** environ;

namespace leatherman { namespace execution {

    void log_execution(string const& file, vector<string> const* arguments);

    const char *const command_shell = "sh";
    const char *const command_args = "-c";

    static uint64_t get_max_descriptor_limit()
    {
        // WARNING: this function is potentially called under vfork
        // See comment below in exec_child in case you're not afraid
#ifdef _SC_OPEN_MAX
        {
            auto open_max = sysconf(_SC_OPEN_MAX);
            if (open_max > 0) {
                return open_max;
            }
        }
#endif  // _SC_OPEN_MAX

#ifdef RLIMIT_NOFILE
        {
            rlimit lim;
            if (getrlimit(RLIMIT_NOFILE, &lim) == 0) {
                return lim.rlim_cur;
            }
        }
#endif  // RLIMIT_NOFILE

#ifdef OPEN_MAX
        return OPEN_MAX;
#else
        return 256;
#endif  // OPEN_MAX
    }

    static volatile bool command_timedout = false;

    static void timer_handler(int signal)
    {
        command_timedout = true;
    }

    string format_error(string const& message, int error)
    {
        if (message.empty()) {
            return _("{1} ({2})", strerror(error), error);
        }
        return _("{1}: {2} ({3}).", message, strerror(error), error);
    }

    static vector<gid_t> get_groups()
    {
        // Query for the number of groups
        auto num = getgroups(0, nullptr);
        if (num < 1) {
            return {};
        }

        // Allocate a buffer for the groups
        vector<gid_t> groups(static_cast<size_t>(num));
        num = getgroups(groups.size(), groups.data());
        if (static_cast<size_t>(num) != groups.size()) {
            return {};
        }
        return groups;
    }

    static bool is_group_member(gid_t gid)
    {
        // Check for primary group
        if (getgid() == gid || getegid() == gid) {
            return true;
        }

        // Get the groups and search for the given gid
        static auto groups = get_groups();
        return find(groups.begin(), groups.end(), gid) != groups.end();
    }

    static bool is_executable(char const* path)
    {
        struct stat fs;
        if (stat(path, &fs) != 0) {
            return false;
        }

        auto euid = geteuid();

        // If effectively running as root, any exec bit will do
        if (euid == 0) {
            return fs.st_mode & (S_IXUSR | S_IXGRP | S_IXOTH);
        }

        // If the file is effectively owned, check for user exec bit
        if (fs.st_uid == euid) {
            return fs.st_mode & S_IXUSR;
        }

        // If the file is owned by a group we're a member of, check for the group exec bit
        if (is_group_member(fs.st_gid)) {
            return fs.st_mode & S_IXGRP;
        }

        // Lastly check for "others" exec bit
        return fs.st_mode & S_IXOTH;
    }

    string which(string const& file, vector<string> const& directories)
    {
        // If the file is already absolute, return it if it's executable
        path p = file;
        boost::system::error_code ec;
        if (p.is_absolute()) {
            return is_regular_file(p, ec) && is_executable(p.c_str()) ? p.string() : string();
        }

        // Otherwise, check for an executable file under the given search paths
        for (auto const& dir : directories) {
            path p = path(dir) / file;
            if (is_regular_file(p, ec) && is_executable(p.c_str())) {
                return p.string();
            }
        }
        return {};
    }

    // Represents information about a pipe
    struct pipe
    {
        // cppcheck-suppress passedByValue
        pipe(string pipe_name, scoped_descriptor desc, function<bool(string const&)> cb) :
            name(move(pipe_name)),
            descriptor(move(desc)),
            callback(move(cb)),
            read(true)
        {
        }

        // cppcheck-suppress passedByValue
        pipe(string pipe_name, scoped_descriptor desc, string buf) :
            name(move(pipe_name)),
            descriptor(move(desc)),
            buffer(move(buf)),
            read(false)
        {
        }

        const string name;
        scoped_descriptor descriptor;
        string buffer;
        function<bool(string const&)> callback;
        bool read;
    };

    static void rw_from_child(pid_t child, array<pipe, 3>& pipes, uint32_t timeout, bool allow_stdin_unread)
    {
        // Each pipe is a tuple of descriptor, buffer to use to read data, and a callback to call when data is read
        // The input pair is a descriptor and text to write to it
        fd_set read_set, write_set;
        while (!command_timedout) {
            FD_ZERO(&read_set);
            FD_ZERO(&write_set);

            // Set up the descriptors and buffers to select upon
            int max = -1;
            for (auto& pipe : pipes) {
                if (pipe.descriptor == -1) {
                    continue;
                }

                FD_SET(pipe.descriptor, pipe.read ? &read_set : &write_set);
                if (pipe.descriptor > max) {
                    max = pipe.descriptor;
                }

                if (pipe.read) {
                    pipe.buffer.resize(4096);
                }
            }

            if (max == -1) {
                // All pipes closed; we're done
                return;
            }

            // If using a timeout, timeout after 500ms to check whether or not the command itself timed out
            timeval read_timeout = {};
            read_timeout.tv_usec = 500000;
            int result = select(max + 1, &read_set, &write_set, nullptr, timeout ? &read_timeout : nullptr);
            if (result == -1) {
                if (errno != EINTR) {
                    throw execution_exception(format_error(_("select call failed waiting for child i/o")));
                }
                // Interrupted by signal
                LOG_DEBUG("select call was interrupted and will be retried.");
                continue;
            } else if (result == 0) {
                // Read timeout, try again
                continue;
            }

            for (auto& pipe : pipes) {
                if (pipe.descriptor == -1 || !FD_ISSET(pipe.descriptor, pipe.read ? &read_set : &write_set)) {
                    continue;
                }

                // There is data to read/write
                auto count = pipe.read ?
                    read(pipe.descriptor, &pipe.buffer[0], pipe.buffer.size()) :
                    write(pipe.descriptor, pipe.buffer.c_str(), pipe.buffer.size());
                if (count < 0) {
                    if (allow_stdin_unread && !pipe.read && errno == EPIPE) {
                        // Input pipe was closed prematurely due to process exit, log and let it go.
                        LOG_DEBUG("{1} pipe i/o was closed early, process may have ignored input.", pipe.name);
                        pipe.descriptor = {};
                        continue;
                    } else if (errno == EINTR) {
                        // Interrupted by signal
                        LOG_DEBUG("{1} pipe i/o was interrupted and will be retried.", pipe.name);
                        continue;
                    }
                    throw execution_exception(_("{1} pipe i/o failed: {2}", pipe.name, format_error()));
                } else if (count == 0) {
                    // Pipe has closed
                    pipe.descriptor = {};
                    continue;
                }

                if (pipe.read) {
                    // Call the callback
                    pipe.buffer.resize(count);
                    if (!pipe.callback(pipe.buffer)) {
                        // Callback signaled that we're done
                        return;
                    }
                } else {
                    // Register written data
                    pipe.buffer.erase(0, count);
                }
            }
        }

        // Should only reach here if the command timed out
        // cppcheck-suppress zerodivcond - http://trac.cppcheck.net/ticket/5402
        throw timeout_exception(_("command timed out after {1} seconds.", timeout), static_cast<size_t>(child));
    }

    static void do_exec_child(int in_fd, int out_fd, int err_fd, uint64_t max_fd, char const* program, char const** argv, char const** envp)
    {
        // WARNING: this function is potentially called from a vfork'd child
        // Do not modify program state from this function; only call setpgid, dup2, close, execve, and _exit
        // Do not allocate heap memory or throw exceptions
        // The child is sharing the address space of the parent process, so carelessly modifying this
        // function may lead to parent state corruption, memory leaks, and/or total protonic reversal
        // As such, strings are explicitly not localized in this function.
        //
        // This is especially important due to a deadlock in vfork/exec on Solaris, identified in
        // http://www.oracle.com/technetwork/server-storage/solaris10/subprocess-136439.html. The solution
        // they use in posix_spawn is to avoid calling functions exported as from libc as global symbols
        // after the fork. `write`, `strlen`, and `close` are still suspect below.

        // Set the process group; this will be used by the parent if we need to kill the process and its children
        if (setpgid(0, 0) == -1) {
            return;
        }

        // Redirect stdin
        if (dup2(in_fd, STDIN_FILENO) == -1) {
            return;
        }

        // Redirect stdout
        if (dup2(out_fd, STDOUT_FILENO) == -1) {
            return;
        }

        // Redirect stderr
        if (dup2(err_fd, STDERR_FILENO) == -1) {
            return;
        }

        // Close all open file descriptors above stderr
        for (uint64_t i = (STDERR_FILENO + 1); i < max_fd; ++i) {
            close(i);
        }

        // Execute the given program; this should not return if successful
        execve(program, const_cast<char* const*>(argv), const_cast<char* const*>(envp));
    }

    void exec_child(int in_fd, int out_fd, int err_fd, uint64_t max_fd, char const* program, char const** argv, char const** envp)
    {
        // WARNING: this function is potentially called from a vfork'd child
        // Do not modify program state from this function; only call setpgid, dup2, close, execve, and _exit
        // Do not allocate heap memory or throw exceptions
        // The child is sharing the address space of the parent process, so carelessly modifying this
        // function may lead to parent state corruption, memory leaks, and/or total protonic reversal

        do_exec_child(in_fd, out_fd, err_fd, max_fd, program, argv, envp);

        // If we've reached here, we've failed, so exit the child
        _exit(errno == 0 ? EXIT_FAILURE : errno);
    }

    // Helper function that turns a vector of strings into a vector of const cstr pointers
    // This is used to pass arguments and environment to execve
    static vector<char const*> to_exec_arg(vector<string> const* argument, string const* first = nullptr)
    {
        vector<char const*> result;
        result.reserve((argument ? argument->size() : 0) + (first ? 1 : 0) + 1 /* terminating null */);
        if (first) {
            result.push_back(first->c_str());
        }
        if (argument) {
            transform(argument->begin(), argument->end(), back_inserter(result), [](string const& s) { return s.c_str(); });
        }
        // Null terminate the list
        result.push_back(nullptr);
        return result;
    }

    // Helper function that creates a vector of environment variables in the format of key=value
    // Also handles merging of environment and defaulting LC_ALL and LANG to C
    static vector<string> create_environment(map<string, string> const* environment, bool merge, bool inherit)
    {
        vector<string> result;

        // Merge in our current environment, if requested
        if (merge && environ) {
            for (auto var = environ; *var; ++var) {
                // Don't respect LC_ALL or LANG from the parent process, unless inherit_locale specified
                if (!inherit && (boost::starts_with(*var, "LC_ALL=") || boost::starts_with(*var, "LANG="))) {
                    continue;
                }
                result.emplace_back(*var);
            }
        }

        // Add the given environment
        if (environment) {
            for (auto const& kvp : *environment) {
                result.emplace_back(_("{1}={2}", kvp.first, kvp.second));
            }
        }

        // Set the locale to C unless specified in the given environment
        string locale_env;
        if (!environment || environment->count("LC_ALL") == 0) {
            if (inherit && environment::get("LC_ALL", locale_env)) {
                result.emplace_back("LC_ALL=" + locale_env);
            } else if (!inherit) {
                result.emplace_back("LC_ALL=C");
            }
        }
        if (!environment || environment->count("LANG") == 0) {
            if (inherit && environment::get("LANG", locale_env)) {
                result.emplace_back("LANG=" + locale_env);
            } else if (!inherit) {
                result.emplace_back("LANG=C");
            }
        }
        return result;
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
        uint32_t timeout)
    {
        // Search for the executable
        string executable = which(file);
        log_execution(executable.empty() ? file : executable, arguments);
        if (executable.empty()) {
            LOG_DEBUG("{1} was not found on the PATH.", file);
            if (options[execution_options::throw_on_nonzero_exit]) {
                throw child_exit_exception(_("child process returned non-zero exit status."), 127, {}, {});
            }
            return {false, "", "", 127, 0};
        }

        // Create the pipes for stdin/stdout redirection
        int pipes[2];
        if (::pipe(pipes) < 0) {
            throw execution_exception(format_error(_("failed to allocate pipe for stdin redirection")));
        }
        scoped_descriptor stdin_read(pipes[0]);
        scoped_descriptor stdin_write(pipes[1]);

        if (::pipe(pipes) < 0) {
            throw execution_exception(format_error(_("failed to allocate pipe for stdout redirection")));
        }
        scoped_descriptor stdout_read(pipes[0]);
        scoped_descriptor stdout_write(pipes[1]);

        // Redirect stderr to stdout, null, or to the pipe to read
        scoped_descriptor stderr_read(-1);
        scoped_descriptor stderr_write(-1);
        scoped_descriptor dev_null(-1);
        int child_stderr = -1;
        if (options[execution_options::redirect_stderr_to_stdout]) {
            child_stderr = stdout_write;
        } else if (options[execution_options::redirect_stderr_to_null]) {
            dev_null = scoped_descriptor(open("/dev/null", O_RDWR));
            child_stderr = dev_null;
        } else {
            if (::pipe(pipes) < 0) {
                throw execution_exception(format_error(_("failed to allocate pipe for stderr redirection")));
            }
            stderr_read = scoped_descriptor(pipes[0]);
            stderr_write = scoped_descriptor(pipes[1]);
            child_stderr = stderr_write;
        }

        // Allocate the child process arguments and envp *before* creating the child
        auto args = to_exec_arg(arguments, &file);
        auto variables = create_environment(environment,
                                            options[execution_options::merge_environment],
                                            options[execution_options::inherit_locale]);
        auto envp = to_exec_arg(&variables);

        // Create the child
        pid_t child = create_child(options,
                                   stdin_read, stdout_write, child_stderr,
                                   get_max_descriptor_limit(),
                                   executable.c_str(), args.data(), envp.data());

        // Close the unused descriptors
        if (!input) {
            stdin_write.release();
        }
        stdin_read.release();
        stdout_write.release();
        stderr_write.release();

        // Define a reaper that is invoked when we exit this scope
        // This ensures that the child won't become a zombie if an exception is thrown
        bool kill_child = true;
        bool success = false;
        bool signaled = false;
        int status = 0;
        scope_exit reaper([&]() {
            if (kill_child) {
                kill(-child, SIGKILL);
            }
            // Wait for the child to exit
            if (waitpid(child, &status, 0) == -1) {
                LOG_DEBUG(format_error(_("waitpid failed")));
                return;
            }
            if (WIFEXITED(status)) {
                status = static_cast<char>(WEXITSTATUS(status));
                success = status == 0;
                return;
            }
            if (WIFSIGNALED(status)) {
                signaled = true;
                status = static_cast<char>(WTERMSIG(status));
                return;
            }
        });

        // Set up an interval timer for timeouts
        // Note: OSX doesn't implement POSIX per-process timers, so we're stuck with the obsolete POSIX timers API
        scope_exit timer_reset;
        if (timeout) {
            struct sigaction sa = {};
            sa.sa_handler = timer_handler;
            if (sigaction(SIGALRM, &sa, nullptr) == -1) {
                throw execution_exception(format_error(_("sigaction failed while setting up timeout")));
            }

            itimerval timer = {};
            timer.it_value.tv_sec = static_cast<decltype(timer.it_interval.tv_sec)>(timeout);
            if (setitimer(ITIMER_REAL, &timer, nullptr) == -1) {
                throw execution_exception(format_error(_("setitimer failed while setting up timeout")));
            }

            // Set the resource to disable the timer
            timer_reset = scope_exit([&]() {
                itimerval timer = {};
                setitimer(ITIMER_REAL, &timer, nullptr);
                command_timedout = false;
            });
        }

        // Execute the PID callback
        if (pid_callback) {
            pid_callback(child);
        }

        // This somewhat complicated construct performs the following:
        // Calls a platform-agnostic implementation of processing stdout/stderr data
        // The platform agnostic code calls back into the given lambda to do the actual reading
        // It provides two callbacks of its own to call when there's data available on stdout/stderr
        // We return from the lambda when all data has been read
        string output, error;
        tie(output, error) = process_streams(options[execution_options::trim_output], stdout_callback, stderr_callback, [&](function<bool(string const&)> const& process_stdout, function<bool(string const&)> const& process_stderr) {
            array<pipe, 3> pipes = { {
                pipe("stdout", move(stdout_read), process_stdout),
                pipe("stderr", move(stderr_read), process_stderr),
                input ? pipe("stdin", move(stdin_write), *input) : pipe("", {}, "")
            }};

            rw_from_child(child, pipes, timeout, options[execution_options::allow_stdin_unread]);
        });

        // Close the read pipes
        // If the child hasn't sent all the data yet, this may signal SIGPIPE on next write
        stdout_read.release();
        stderr_read.release();

        // Wait for the child to exit
        kill_child = false;
        reaper.invoke();

        if (signaled) {
            LOG_DEBUG("process was signaled with signal {1}.", status);
        } else {
            LOG_DEBUG("process exited with status code {1}.", status);
        }

        // Throw exception if needed
        if (!success) {
            if (!signaled && status != 0 && options[execution_options::throw_on_nonzero_exit]) {
                throw child_exit_exception(_("child process returned non-zero exit status ({1}).", status), status, move(output), move(error));
            }
            if (signaled && options[execution_options::throw_on_signal]) {
                throw child_signal_exception(_("child process was terminated by signal ({1}).", status), status, move(output), move(error));
            }
        }
        return {success, move(output), move(error), status, static_cast<size_t>(child)};
    }

}}  // namespace leatherman::execution

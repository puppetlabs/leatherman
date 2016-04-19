#include <leatherman/execution/execution.hpp>
#include <leatherman/util/environment.hpp>
#include <leatherman/util/scope_exit.hpp>
#include <leatherman/util/windows/scoped_handle.hpp>
#include <leatherman/util/scoped_env.hpp>
#include <leatherman/windows/system_error.hpp>
#include <leatherman/windows/windows.hpp>
#include <leatherman/logging/logging.hpp>
#include <boost/filesystem.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/nowide/convert.hpp>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <cstdlib>
#include <cstdio>
#include <sstream>
#include <cstring>
#include <random>

using namespace std;
using namespace leatherman::windows;
using namespace leatherman::logging;
using namespace leatherman::util;
using namespace leatherman::util::windows;
using namespace boost::filesystem;
using namespace boost::algorithm;
namespace lth_locale = leatherman::locale;

namespace leatherman { namespace execution {

    void log_execution(string const& file, vector<string> const* arguments);

    const char *const command_shell = "cmd.exe";
    const char *const command_args = "/c";

    struct extpath_helper
    {
        vector<string> const& ext_paths() const
        {
            return _extpaths;
        }

        bool contains(const string & ext) const
        {
            return binary_search(_extpaths.begin(), _extpaths.end(), to_lower_copy(ext));
        }

     private:
        // Use sorted, lower-case operations to ignore case and use binary search.
        vector<string> _extpaths = {".bat", ".cmd", ".com", ".exe"};;
    };

    static bool is_executable(path const& p, extpath_helper const* helper = nullptr)
    {
        // If there's an error accessing file status, we assume is_executable
        // is false and return. The reason for failure doesn't matter to us.
        boost::system::error_code ec;
        bool isfile = is_regular_file(p, ec);
        if (ec) {
            LOG_TRACE("error reading status of path {1}: {2} ({3})", p, ec.message(), ec.value());
        }

        if (helper) {
            // Checking extensions aren't needed if we explicitly specified it.
            // If helper was passed, then we haven't and should check the ext.
            isfile &= helper->contains(p.extension().string());
        }
        return isfile;
    }

    string which(string const& file, vector<string> const& directories)
    {
        // On Windows, everything has execute permission; Ruby determined
        // executability based on extension {com, exe, bat, cmd}. We'll do the
        // same check here using extpath_helper.
        static extpath_helper helper;

        // If the file is already absolute, return it if it's executable.
        path p = file;
        if (p.is_absolute()) {
            return is_executable(p, &helper) ? p.string() : string();
        }

        // On Windows, treat 'echo' as a command that can be found
        if (file == "echo") {
            return "echo";
        }

        // Otherwise, check for an executable file under the given search paths
        for (auto const& dir : directories) {
            path p = path(dir) / file;
            if (!p.has_extension()) {
                path pext = p;
                for (auto const&ext : helper.ext_paths()) {
                    pext.replace_extension(ext);
                    if (is_executable(pext)) {
                        return pext.string();
                    }
                }
            }
            if (is_executable(p, &helper)) {
                return p.string();
            }
        }
        return {};
    }

    // Create a pipe, throwing if there's an error. Returns {read, write} handles.
    // Always creates overlapped pipes.
    static tuple<scoped_handle, scoped_handle> CreatePipeThrow()
    {
        static LONG counter = 0;
        static boost::uuids::random_generator rand_uuid;

        SECURITY_ATTRIBUTES attributes = {};
        attributes.nLength = sizeof(SECURITY_ATTRIBUTES);
        attributes.bInheritHandle = TRUE;
        attributes.lpSecurityDescriptor = NULL;

        // Format a name for the pipe. Use the thread id to ensure no two threads try to use the same
        // pipe, and a counter to generate multiple pipes for the same process invocation.
        // A scenario exists using timeouts where we could release the invoking end of a named pipe
        // but the other end doesn't release. Then the invoking thread shuts down and another with
        // the same thread id is started and reconnects to the existing named pipe. Use the process
        // id and a random UUID to make that highly unlikely.
        wstring name = boost::nowide::widen(lth_locale::format("\\\\.\\Pipe\\leatherman.{1}.{2}.{3}.{4}",
            GetCurrentProcessId(),
            GetCurrentThreadId(),
            InterlockedIncrement(&counter),
            to_string(rand_uuid())));

        // Create the read pipe
        scoped_handle read_handle(CreateNamedPipeW(
            name.c_str(),
            PIPE_ACCESS_INBOUND | FILE_FLAG_OVERLAPPED,
            PIPE_TYPE_BYTE | PIPE_WAIT,
            1,
            4096,
            4096,
            0,
            &attributes));

        if (read_handle == INVALID_HANDLE_VALUE) {
            LOG_ERROR("failed to create read pipe: {1}.", windows::system_error());
            throw execution_exception("failed to create read pipe.");
        }

        // Open the write pipe
        scoped_handle write_handle(CreateFileW(
            name.c_str(),
            GENERIC_WRITE,
            0,
            &attributes,
            OPEN_EXISTING,
            FILE_ATTRIBUTE_NORMAL,
            nullptr));

        if (write_handle == INVALID_HANDLE_VALUE) {
            LOG_ERROR("failed to create write pipe: {1}.", windows::system_error());
            throw execution_exception("failed to create write pipe.");
        }

        return make_tuple(move(read_handle), move(write_handle));
    }

    // Source: http://blogs.msdn.com/b/twistylittlepassagesallalike/archive/2011/04/23/everyone-quotes-arguments-the-wrong-way.aspx
    static string ArgvToCommandLine(vector<string> const& arguments, bool preserve = false)
    {
        // Unless we're told otherwise, don't quote unless we actually need to do so - hopefully avoid problems if
        // programs won't parse quotes properly.
        string commandline;
        for (auto const& arg : arguments) {
            if (arg.empty()) {
                continue;
            } else if (preserve || arg.find_first_of(" \t\n\v\"") == arg.npos) {
                commandline += arg;
            } else {
                commandline += '"';
                for (auto it = arg.begin(); ; ++it) {
                    unsigned num_back_slashes = 0;
                    while (it != arg.end() && *it == '\\') {
                        ++it;
                        ++num_back_slashes;
                    }

                    if (it == arg.end()) {
                        // Escape all backslashes, but let the terminating double quotation mark we add below be
                        // interpreted as a metacharacter.
                        commandline.append(num_back_slashes * 2, '\\');
                        break;
                    } else if (*it == '"') {
                        // Escape all backslashes and the following double quotation mark.
                        commandline.append(num_back_slashes * 2 + 1, '\\');
                        commandline.push_back(*it);
                    } else {
                        // Backslashes aren't special here.
                        commandline.append(num_back_slashes, '\\');
                        commandline.push_back(*it);
                    }
                }
                commandline += '"';
            }
            commandline += ' ';
        }

        // Strip the trailing space.
        boost::trim_right(commandline);
        return commandline;
    }

    // Represents information about a pipe
    struct pipe
    {
        pipe(string pipe_name, scoped_handle pipe_handle, function<bool(string const&)> cb) :
            name(move(pipe_name)),
            handle(move(pipe_handle)),
            overlapped{},
            pending(false),
            read(true),
            callback(move(cb))
        {
            init();
        }

        pipe(string pipe_name, scoped_handle pipe_handle, string buf) :
            name(move(pipe_name)),
            handle(move(pipe_handle)),
            overlapped{},
            pending(false),
            read(false),
            buffer(move(buf))
        {
            init();
        }

        const string name;
        scoped_handle handle;
        OVERLAPPED overlapped;
        scoped_handle event;
        bool pending;
        bool read;
        string buffer;
        function<bool(string const&)> callback;

     private:
        void init()
        {
            if (handle != INVALID_HANDLE_VALUE) {
                event = scoped_handle(CreateEvent(nullptr, TRUE, FALSE, nullptr));
                if (!event) {
                    LOG_ERROR("failed to create {1} read event: {2}.", name, windows::system_error());
                    throw execution_exception("failed to create read event.");
                }
                overlapped.hEvent = event;
            }
        }
    };

    static void rw_from_child(DWORD child, array<pipe, 3>& pipes, uint32_t timeout, HANDLE timer)
    {
        vector<HANDLE> wait_handles;
        while (true)
        {
            // Process all pipes
            for (auto& pipe : pipes) {
                // If the handle is closed or is pending, skip
                if (pipe.handle == INVALID_HANDLE_VALUE || pipe.pending) {
                    continue;
                }

                // Process the pipe until pending
                while (true) {
                    // Before doing anything, check to see if there's been a timeout
                    // This is done pre-emptively in case ReadFile never returns ERROR_IO_PENDING
                    if (timeout && WaitForSingleObject(timer, 0) == WAIT_OBJECT_0) {
                        throw timeout_exception(lth_locale::format("command timed out after {1} seconds.", timeout), static_cast<size_t>(child));
                    }

                    if (pipe.read) {
                        // Read the data
                        pipe.buffer.resize(4096);
                    }

                    DWORD count = 0;
                    auto success = pipe.read ?
                        ReadFile(pipe.handle, &pipe.buffer[0], pipe.buffer.size(), &count, &pipe.overlapped) :
                        WriteFile(pipe.handle, pipe.buffer.c_str(), pipe.buffer.size(), &count, &pipe.overlapped);
                    if (!success) {
                        // Treat broken pipes as closed pipes
                        if (GetLastError() == ERROR_BROKEN_PIPE) {
                            pipe.handle = {};
                            break;
                        }
                        // Check to see if it's a pending operation
                        if (GetLastError() == ERROR_IO_PENDING) {
                            pipe.pending = true;
                            break;
                        }
                        LOG_ERROR("{1} pipe i/o failed: {2}.", pipe.name, windows::system_error());
                        throw execution_exception("child i/o failed.");
                    }

                    // Check for closed pipe
                    if (count == 0) {
                        pipe.handle = {};
                        break;
                    }

                    if (pipe.read) {
                        // Read completed immediately, process the data
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

            // All pipes should be pending now
            wait_handles.clear();
            for (auto const& pipe : pipes) {
                if (pipe.handle == INVALID_HANDLE_VALUE || !pipe.pending) {
                    continue;
                }
                wait_handles.push_back(pipe.event);
            }

            // If no wait handles, then we're done processing
            if (wait_handles.empty()) {
                return;
            }

            if (timeout) {
                wait_handles.push_back(timer);
            }

            // Wait for data (and, optionally, timeout)
            auto result = WaitForMultipleObjects(wait_handles.size(), wait_handles.data(), FALSE, INFINITE);
            if (result >= (WAIT_OBJECT_0 + wait_handles.size())) {
                LOG_ERROR("failed to wait for child process i/o: {1}.", windows::system_error());
                throw execution_exception("failed to wait for child process i/o.");
            }

            // Check for timeout
            DWORD index = result - WAIT_OBJECT_0;
            if (timeout && wait_handles[index] == timer) {
                throw timeout_exception(lth_locale::format("command timed out after {1} seconds.", timeout), static_cast<size_t>(child));
            }

            // Find the pipe for the event that was signalled
            for (auto& pipe : pipes) {
                if (pipe.handle == INVALID_HANDLE_VALUE || !pipe.pending || pipe.event != wait_handles[index]) {
                    continue;
                }

                // Pipe is no longer pending
                pipe.pending = false;

                // Get the overlapped result and process it
                DWORD count = 0;
                if (!GetOverlappedResult(pipe.handle, &pipe.overlapped, &count, FALSE)) {
                    if (GetLastError() != ERROR_BROKEN_PIPE) {
                        LOG_ERROR("asynchronous i/o on {1} failed: {2}.", pipe.name, windows::system_error());
                        throw execution_exception("asynchronous i/o failed.");
                    }
                    // Treat a broken pipe as nothing left to read
                    count = 0;
                }
                // Check for closed pipe
                if (count == 0) {
                    pipe.handle = {};
                    break;
                }

                if (pipe.read) {
                    // Read completed, process the data
                    pipe.buffer.resize(count);
                    if (!pipe.callback(pipe.buffer)) {
                        // Callback signaled that we're done
                        return;
                    }
                } else {
                    // Register written data
                    pipe.buffer.erase(0, count);
                }
                break;
            }
        }
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
        // Since we use a job object in the windows world, we want to
        // be sure we're not in a job object, or at least able to
        // break our processes out if we are in one.
        BOOL in_job;
        bool use_job_object = true;
        if (!IsProcessInJob(GetCurrentProcess(), nullptr, &in_job)) {
            throw execution_exception("could not determine if the parent process is running in a job object");
        }
        if (in_job) {
            JOBOBJECT_BASIC_LIMIT_INFORMATION limits;
            if (!QueryInformationJobObject(nullptr, JobObjectBasicLimitInformation, &limits, sizeof(limits), nullptr)
                || !(limits.LimitFlags & JOB_OBJECT_LIMIT_BREAKAWAY_OK)) {  // short-circuits if QueryInformationJobObject fails
                use_job_object = false;
            }
        }

        // Search for the executable
        string executable = which(file);
        log_execution(executable.empty() ? file : executable, arguments);
        if (executable.empty()) {
            LOG_DEBUG("{1} was not found on the PATH.", file);
            if (options[execution_options::throw_on_nonzero_exit]) {
                throw child_exit_exception("child process returned non-zero exit status.", 127, {}, {});
            }
            return {false, "", "", 127, 0};
        }

        // Setup the execution environment
        vector<char> modified_environ;
        vector<scoped_env> scoped_environ;
        if (options[execution_options::merge_environment]) {
            // Modify the existing environment, then restore it after. There's no way to modify environment variables
            // after the child has started. An alternative would be to use GetEnvironmentStrings and add/modify the block,
            // but searching for and modifying existing environment strings to update would be cumbersome in that form.
            // See http://msdn.microsoft.com/en-us/library/windows/desktop/ms682009(v=vs.85).aspx
            if (!environment || environment->count("LC_ALL") == 0) {
                scoped_environ.emplace_back("LC_ALL", "C");
            }
            if (!environment || environment->count("LANG") == 0) {
                scoped_environ.emplace_back("LANG", "C");
            }
            if (environment) {
                for (auto const& kv : *environment) {
                    // Use scoped_env to save the old state and restore it on return.
                    LOG_DEBUG("child environment {1}={2}", kv.first, kv.second);
                    scoped_environ.emplace_back(kv.first, kv.second);
                }
            }
        } else {
            // We aren't inheriting the environment, so create an environment block instead of changing existing env.
            // Environment variables must be sorted alphabetically and case-insensitive,
            // so copy them all into the same map with case-insensitive key compare:
            //   http://msdn.microsoft.com/en-us/library/windows/desktop/ms682009(v=vs.85).aspx
            map<string, string, bool(*)(string const&, string const&)> sortedEnvironment(
                [](string const& a, string const& b) { return ilexicographical_compare(a, b); });
            if (environment) {
                sortedEnvironment.insert(environment->begin(), environment->end());
            }

            // Insert LANG and LC_ALL if they aren't already present. Emplace ensures this behavior.
            sortedEnvironment.emplace("LANG", "C");
            sortedEnvironment.emplace("LC_ALL", "C");

            // An environment block is a NULL-terminated list of NULL-terminated strings.
            for (auto const& variable : sortedEnvironment) {
                LOG_DEBUG("child environment {1}={2}", variable.first, variable.second);
                string var = variable.first + "=" + variable.second;
                for (auto c : var) {
                    modified_environ.push_back(c);
                }
                modified_environ.push_back('\0');
            }
            modified_environ.push_back('\0');
        }

        // Execute the command, reading the results into a buffer until there's no more to read.
        // See http://msdn.microsoft.com/en-us/library/windows/desktop/ms682499(v=vs.85).aspx
        // for details on redirecting input/output.
        scoped_handle stdInRd, stdInWr;
        tie(stdInRd, stdInWr) = CreatePipeThrow();
        if (!SetHandleInformation(stdInWr, HANDLE_FLAG_INHERIT, 0)) {
            throw execution_exception("pipe could not be modified");
        }

        scoped_handle stdOutRd, stdOutWr;
        tie(stdOutRd, stdOutWr) = CreatePipeThrow();
        if (!SetHandleInformation(stdOutRd, HANDLE_FLAG_INHERIT, 0)) {
            throw execution_exception("pipe could not be modified");
        }

        scoped_handle stdErrRd, stdErrWr;
        if (!options[execution_options::redirect_stderr_to_stdout]) {
            // If redirecting to null, open the "NUL" device and inherit the handle
            if (options[execution_options::redirect_stderr_to_null]) {
                SECURITY_ATTRIBUTES attributes = {};
                attributes.nLength = sizeof(SECURITY_ATTRIBUTES);
                attributes.bInheritHandle = TRUE;
                stdErrWr = scoped_handle(CreateFileW(L"nul", GENERIC_WRITE, FILE_SHARE_WRITE, &attributes, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr));
                if (stdErrWr == INVALID_HANDLE_VALUE) {
                    throw execution_exception("cannot open NUL device for redirecting stderr.");
                }
            } else {
                // Otherwise, we're reading from stderr, so create a pipe
                tie(stdErrRd, stdErrWr) = CreatePipeThrow();
                if (!SetHandleInformation(stdErrRd, HANDLE_FLAG_INHERIT, 0)) {
                    throw execution_exception("pipe could not be modified");
                }
            }
        }

        // Execute the command with arguments. Prefix arguments with the executable, or quoted arguments won't work.
        auto commandLine = arguments ?
            boost::nowide::widen(ArgvToCommandLine({ executable }) + " " + ArgvToCommandLine(*arguments, options[execution_options::preserve_arguments])) : L"";

        STARTUPINFO startupInfo = {};
        startupInfo.cb = sizeof(startupInfo);
        startupInfo.dwFlags |= STARTF_USESTDHANDLES;
        startupInfo.hStdInput = stdInRd;
        startupInfo.hStdOutput = stdOutWr;

        // Set up stderr redirection to out or the pipe (which may be INVALID_HANDLE_VALUE, i.e. "null")
        if (options[execution_options::redirect_stderr_to_stdout]) {
            startupInfo.hStdError = stdOutWr;
        } else {
            startupInfo.hStdError = stdErrWr;
        }

        PROCESS_INFORMATION procInfo = {};

        // Set up flags for CreateProcess based on whether the create_new_process_group
        // option was set and the parent process is running in a Job object.
        auto creation_flags = CREATE_NO_WINDOW;

        if (use_job_object) {
            creation_flags |= CREATE_BREAKAWAY_FROM_JOB;
        }
        if (options[execution_options::create_new_process_group]) {
            creation_flags |= CREATE_NEW_PROCESS_GROUP;
        }

        if (!CreateProcessW(
            boost::nowide::widen(executable).c_str(),
            &commandLine[0], /* Pass a modifiable string buffer; the contents may be modified */
            NULL,           /* Don't allow child process to inherit process handle */
            NULL,           /* Don't allow child process to inherit thread handle */
            TRUE,           /* Inherit handles from the calling process for communication */
            creation_flags,
            options[execution_options::merge_environment] ? NULL : modified_environ.data(),
            NULL,           /* Use existing current directory */
            &startupInfo,   /* STARTUPINFO for child process */
            &procInfo)) {   /* PROCESS_INFORMATION pointer for output */
            LOG_ERROR("failed to create process: {1}.", windows::system_error());
            throw execution_exception("failed to create child process.");
        }

        // Release unused pipes, to avoid any races in process completion.
        if (!input) {
            stdInWr.release();
        }
        stdInRd.release();
        stdOutWr.release();
        stdErrWr.release();

        scoped_handle hProcess(procInfo.hProcess);
        scoped_handle hThread(procInfo.hThread);

        // Use a Job Object to group any child processes spawned by the CreateProcess invocation, so we can
        // easily stop them in case of a timeout.
        bool create_job_object = use_job_object && !options[execution_options::create_new_process_group];
        scoped_handle hJob;
        if (create_job_object) {
            hJob = scoped_handle(CreateJobObjectW(nullptr, nullptr));
            if (hJob == NULL) {
                LOG_ERROR("failed to create job object: {1}.", windows::system_error());
                throw execution_exception("failed to create job object.");
            } else if (!AssignProcessToJobObject(hJob, hProcess)) {
                LOG_ERROR("failed to associate process with job object: {1}.", windows::system_error());
                throw execution_exception("failed to associate process with job object.");
            }
        }

        bool terminate = true;
        scope_exit reaper([&]() {
            if (terminate) {
                // Terminate the process on an exception
                if (create_job_object) {
                    if (!TerminateJobObject(hJob, -1)) {
                        LOG_ERROR("failed to terminate process: {1}.", windows::system_error());
                    }
                } else {
                    LOG_WARNING("could not terminate process {1} because a job object could not be used.", procInfo.dwProcessId);
                }
            }
        });

        // Create a waitable timer if given a timeout
        scoped_handle timer;
        if (timeout) {
            timer = scoped_handle(CreateWaitableTimer(nullptr, TRUE, nullptr));
            if (!timer) {
                LOG_ERROR("failed to create waitable timer: {1}.", windows::system_error());
                throw execution_exception("failed to create waitable timer.");
            }

            // "timeout" in X intervals in the future (1 interval = 100 ns)
            // The negative value indicates relative to the current time
            LARGE_INTEGER future;
            future.QuadPart = timeout * -10000000ll;
            if (!SetWaitableTimer(timer, &future, 0, nullptr, nullptr, FALSE)) {
                LOG_ERROR("failed to set waitable timer: {1}.", windows::system_error());
                throw execution_exception("failed to set waitable timer.");
            }
        }

        // Execute the PID callback
        if (pid_callback) {
            pid_callback(procInfo.dwProcessId);
        }

        string output, error;
        tie(output, error) = process_streams(options[execution_options::trim_output], stdout_callback, stderr_callback, [&](function<bool(string const&)> const& process_stdout, function<bool(string const&)> const& process_stderr) {
            // Read the child output
            array<pipe, 3> pipes = { {
                input ? pipe("stdin", move(stdInWr), *input) : pipe("stdin", {}, ""),
                pipe("stdout", move(stdOutRd), process_stdout),
                pipe("stderr", move(stdErrRd), process_stderr)
            } };

            rw_from_child(procInfo.dwProcessId, pipes, timeout, timer);
        });

        HANDLE handles[2] = { hProcess, timer };
        auto wait_result = WaitForMultipleObjects(timeout ? 2 : 1, handles, FALSE, INFINITE);
        if (wait_result == WAIT_OBJECT_0) {
            // Process has terminated
            terminate = false;
        } else if (wait_result == WAIT_OBJECT_0 + 1) {
            // Timeout while waiting on the process to complete
            throw timeout_exception(lth_locale::format("command timed out after {1} seconds.", timeout), static_cast<size_t>(procInfo.dwProcessId));
        } else {
            LOG_ERROR("failed to wait for child process to terminate: {1}.", windows::system_error());
            throw execution_exception("failed to wait for child process to terminate.");
        }

        // Now check the process return status.
        DWORD exit_code;
        if (!GetExitCodeProcess(hProcess, &exit_code)) {
            throw execution_exception("error retrieving exit code of completed process");
        }

        LOG_DEBUG("process exited with exit code {1}.", exit_code);

        if (exit_code != 0 && options[execution_options::throw_on_nonzero_exit]) {
            throw child_exit_exception("child process returned non-zero exit status.", exit_code, output, error);
        }
        return {exit_code == 0, move(output), move(error), static_cast<int>(exit_code), static_cast<size_t>(procInfo.dwProcessId)};
    }

}}  // namespace leatherman::execution

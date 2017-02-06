#include <leatherman/execution/execution.hpp>
#include <leatherman/locale/locale.hpp>
#include "../platform.hpp"

// Mark string for translation (alias for leatherman::locale::format)
using leatherman::locale::_;

namespace leatherman { namespace execution {

    pid_t create_child(leatherman::util::option_set<execution_options> const& options,
                       int in_fd, int out_fd, int err_fd, uint64_t max_fd,
                       char const* program, char const** argv, char const** envp)
    {
        // Fork the child process
        // Note: this uses vfork (unless the thread_safe execution option is specified), which is inherently unsafe
        // (the parent's address space is shared with the child)
        pid_t pid = options[execution_options::thread_safe] ? fork() : vfork();
        if (pid < 0) {
            throw execution_exception(format_error(_("failed to fork child process")));
        }

        if (pid == 0) {  // Is this the child process?
            // Exec the child; this never returns
            exec_child(in_fd, out_fd, err_fd, max_fd, program, argv, envp);
        }

        return pid;
    }

}}  // namespace leatherman::execution

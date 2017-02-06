#include <unistd.h>
#include <errno.h>

namespace leatherman { namespace execution {

    std::string format_error(std::string const& message = std::string(), int error = errno);

    pid_t create_child(leatherman::util::option_set<execution_options> const& options,
                       int in_fd, int out_fd, int err_fd, uint64_t max_fd,
                       char const* program, char const** argv, char const** envp);

    void exec_child(int in_fd, int out_fd, int err_fd, uint64_t max_fd,
                    char const* program, char const** argv, char const** envp);

}}  // namespace leatherman::execution

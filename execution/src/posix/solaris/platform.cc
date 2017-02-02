/*
 * Portions of this file were copied from OpenSSH's
 * openbsd-compat/port-solaris.c file, that file
 * contained the following copyright notice:
 *
 * Copyright (c) 2006 Chad Mynhier.
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#include <leatherman/execution/execution.hpp>
#include <leatherman/locale/locale.hpp>
#include <boost/format.hpp>

#include <fcntl.h>
#include <libcontract.h>
#include <sys/ctfs.h>
#include <sys/contract/process.h>

#include "../platform.hpp"

#define CT_TEMPLATE CTFS_ROOT "/process/template"
#define CT_LATEST CTFS_ROOT "/process/latest"

// Mark string for translation (alias for leatherman::locale::format)
using leatherman::locale::_;

using namespace std;

namespace leatherman { namespace execution {

    static int activate_new_contract_template(void)
    {
        int tmpl_fd;
        int err;

        // Open a template
        if ((tmpl_fd = open(CT_TEMPLATE, O_RDWR)) == -1) {
            err = errno;
            goto fail;
        }

        // Set the template parameters and event sets
        if ((err = ct_pr_tmpl_set_param(tmpl_fd, CT_PR_PGRPONLY)) != 0) {
            goto close_fail;
        }
        if ((err = ct_pr_tmpl_set_fatal(tmpl_fd, CT_PR_EV_HWERR)) != 0) {
            goto close_fail;
        }
        if ((err = ct_tmpl_set_critical(tmpl_fd, 0)) != 0) {
            goto close_fail;
        }
        if ((err = ct_tmpl_set_informative(tmpl_fd, CT_PR_EV_HWERR)) != 0) {
            goto close_fail;
        }

        // Now make this the active template for this process
        if ((err = ct_tmpl_activate(tmpl_fd)) != 0) {
            goto close_fail;
        }

        return tmpl_fd;

    close_fail:
        close(tmpl_fd);
    fail:
        throw execution_exception(format_error(_("failed to create process contract template"), err));
    }

    static int deactivate_contract_template(int tmpl_fd)
    {
        // WARNING: this function is potentially called from a vfork'd child
        // Do not modify program state from this function; only call setpgid, dup2, close, execve, and _exit
        // Do not allocate heap memory or throw exceptions
        // The child is sharing the address space of the parent process, so carelessly modifying this
        // function may lead to parent state corruption, memory leaks, and/or total protonic reversal

        if (tmpl_fd < 0) {
            return 0;
        }

        // Deactivate the template
        int err = ct_tmpl_clear(tmpl_fd);

        close(tmpl_fd);

        return err;
    }

    // Lookup the latest child process contract ID
    static ctid_t get_latest_child_contract_id(void)
    {
        int stat_fd;
        ct_stathdl_t stathdl;
        ctid_t ctid;
        int err;

        if ((stat_fd = open(CT_LATEST, O_RDONLY)) < 0) {
            err = errno;
            goto fail;
        }

        if ((err = ct_status_read(stat_fd, CTD_COMMON, &stathdl)) != 0) {
            close(stat_fd);
            goto fail;
        }

        ctid = ct_status_get_id(stathdl);
        err = errno;

        ct_status_free(stathdl);
        close(stat_fd);

        if (ctid < 0) {
            goto fail;
        }

        return ctid;

    fail:
        throw execution_exception(format_error(_("failed to lookup the latest child process contract"), err));
    }

    static void abandon_latest_child_contract()
    {
        ctid_t ctid = get_latest_child_contract_id();
        int ctl_fd;
        int err;

        if ((ctl_fd = open((boost::format { "%s/process/%d/ctl" } % CTFS_ROOT % ctid).str().c_str(), O_WRONLY)) < 0) {
            err = errno;
            goto fail;
        }

        // Abandon the contract created for the child process
        err = ct_ctl_abandon(ctl_fd);

        close(ctl_fd);

        if (err == 0) {
            return;
        }

    fail:
        throw execution_exception(format_error(_("failed to abandon contract created for a child process"), err));
    }

    pid_t create_child(leatherman::util::option_set<execution_options> const& options,
                       int in_fd, int out_fd, int err_fd, uint64_t max_fd,
                       char const* program, char const** argv, char const** envp)
    {
        bool detach = options[execution_options::create_detached_process];
        // Create a new process contract template & activate it
        int tmpl_fd = detach ? activate_new_contract_template() : -1;
        int err;

        // Fork the child process
        // Note: this uses vfork (unless the thread_safe execution option is specified), which is inherently unsafe
        // (the parent's address space is shared with the child)
        pid_t pid = options[execution_options::thread_safe] ? fork() : vfork();
        if (pid < 0) {
            err = errno;
            deactivate_contract_template(tmpl_fd);
            throw execution_exception(format_error(_("failed to fork child process"), err));
        }

        if (pid == 0) {  // Is this the child process?
            if ((err = deactivate_contract_template(tmpl_fd)) != 0) {
                _exit(err);
            }
            // Exec the child; this never returns
            exec_child(in_fd, out_fd, err_fd, max_fd, program, argv, envp);
        }

        // This is the parent process
        if ((err = deactivate_contract_template(tmpl_fd)) != 0) {
            throw execution_exception(format_error(_("failed to deactivate contract template created for a child process"), err));
        }
        if (detach) {
            // Abandon the contract created for the child process
            abandon_latest_child_contract();
        }

        return pid;
    }

}}  // namespace leatherman::execution

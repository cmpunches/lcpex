#ifndef LCPEX_LIBLCPEX_H
#define LCPEX_LIBLCPEX_H

#include <cstdio>
#include <cstdlib>
#include <poll.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/wait.h>
#include <iostream>
#include "string_expansion/string_expansion.h"
#include "helpers.h"
#include "Contexts.h"
#include "vpty/pty_fork_mod/tty_functions.h"
#include "vpty/pty_fork_mod/pty_fork.h"
#include "vpty/libclpex_tty.h"

// execute
// uses fork/pipe/dup2/execvp using only pipes for i/o stream redirection
// this will tee output to the parent stdout/stderr in addition to respective
// log paths specified as arguments
// note: do not call directly, use lcpex
int execute(
        std::string command,
        std::string stdout_log_file,
        std::string stderr_log_file,
        bool context_override,
        std::string context_user,
        std::string context_group,
        bool environment_supplied
);


// execute a command and capture its exit code, tee its stdout/stderr to log files
// and to parent's stdout/stderr, with explicit user/group context
int lcpex(
        std::string command,
        std::string stdout_log_file,
        std::string stderr_log_file,
        bool context_override,
        std::string context_user,
        std::string context_group,
        bool force_pty,
        bool is_shell_command,
        std::string shell_path,
        std::string shell_execution_arg,
        bool supply_environment,
        std::string shell_source_subcommand,
        std::string environment_file_path
);


std::string prefix_generator(
        std::string command,
        bool is_shell_command,
        std::string shell_path,
        std::string shell_execution_arg,
        bool supply_environment,
        std::string shell_source_subcommand,
        std::string environment_file_path
);

#endif //LCPEX_LIBLCPEX_H

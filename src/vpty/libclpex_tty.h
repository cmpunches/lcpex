//
// Created by phanes on 2/6/23.
//

#ifndef LCPEX_LIBCLPEX_TTY_H
#define LCPEX_LIBCLPEX_TTY_H

#include <cstdio>
#include <cstdlib>
#include <poll.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/wait.h>
#include <iostream>
#include "../string_expansion/string_expansion.h"
#include "../helpers.h"
#include "../Contexts.h"
#include "../vpty/pty_fork_mod/tty_functions.h"
#include "../vpty/pty_fork_mod/pty_fork.h"
#include <sys/ioctl.h>
#include <string>

int exec_pty(
        std::string command,
        std::string stdout_log_file,
        std::string stderr_log_file,
        bool context_override,
        std::string context_user,
        std::string context_group,
        bool environment_supplied
);


#endif //LCPEX_LIBCLPEX_TTY_H

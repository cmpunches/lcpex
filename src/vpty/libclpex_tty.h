#ifndef LCPEX_LIBCLPEX_TTY_H
#define LCPEX_LIBCLPEX_TTY_H

#include "./pty_fork_mod/pty_master_open.h"
#include "./pty_fork_mod/tty_functions.h"
#include "./pty_fork_mod/pty_fork.h"
#include "../string_expansion.h"
#include "../helpers.h"
#include <iostream>
#include <cstdio>
#include <asm-generic/ioctls.h>
#include <sys/ioctl.h>
#include <termios.h>
#include <sys/wait.h>
#include <pty.h>
#include <poll.h>
#include <fcntl.h>



// execute_pty
// same as execute, using extraction from pty_fork
// creates a slave vpty for child spawning and routes i/o in the same manner between parent
// and slave
// ref: https://man7.org/tlpi/code/online/dist/pty/pty_fork.c.html
int execute_pty( std::string command, std::string stdout_log_file, std::string stderr_log_file );

#endif //LCPEX_LIBCLPEX_TTY_H

#ifndef LCPEX_PTY_FORK_H
#define LCPEX_PTY_FORK_H

#include <sys/types.h>
#include "./pty_master_open.h"
#include <cstdio>
#include "./tty_functions.h"
#include <sys/ioctl.h>


#define MAX_SNAME 1000                  /* Maximum size for pty slave name */

pid_t ptyFork(int *masterFd, char *slaveName, size_t snLen,
              const struct termios *slaveTermios, const struct winsize *slaveWS);


#endif //LCPEX_PTY_FORK_H

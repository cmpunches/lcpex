#ifndef LCPEX_PTY_MASTER_OPEN_H
#define LCPEX_PTY_MASTER_OPEN_H


/* pty_open.h

   Header file for pty_open.c (and pty_master_open_bsd.c).
*/
#include <sys/types.h>
#include <errno.h>
#include <cstdlib>
#include <fcntl.h>
#include <csignal>
#include <cstring>
#include <stdlib.h>
#include <fcntl.h>

int ptyMasterOpen(char *slaveName, size_t snLen);

#endif //LCPEX_PTY_MASTER_OPEN_H

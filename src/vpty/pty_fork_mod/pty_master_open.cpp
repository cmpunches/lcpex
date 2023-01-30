#include "pty_master_open.h"

/* pty_master_open.c

   Implement our ptyMasterOpen() function, based on UNIX 98 pseudoterminals.
   See comments below.

   See also pty_master_open_bsd.c.
*/

/* Open a vpty master, returning file descriptor, or -1 on error.
   On successful completion, the name of the corresponding vpty
   slave is returned in 'slaveName'. 'snLen' should be set to
   indicate the size of the buffer pointed to by 'slaveName'. */

int ptyMasterOpen()
{
    int masterFd, savedErrno;
    char *p;

    /* Open vpty master */
    masterFd = posix_openpt(O_RDWR | O_NOCTTY);

    if (masterFd == -1)
        return -1;

    /* Grant access to slave vpty */
    if ( grantpt(masterFd) == -1 ) {
        savedErrno = errno;

        /* Might change 'errno' */
        close( masterFd );

        errno = savedErrno;
        return -1;
    }

    /* Unlock slave vpty */
    if (unlockpt(masterFd) == -1) {
        savedErrno = errno;

        /* Might change 'errno' */
        close(masterFd);

        errno = savedErrno;
        return -1;
    }

    /* Get slave vpty name */
    p = ptsname(masterFd);
    if (p == nullptr) {
        savedErrno = errno;

        /* Might change 'errno' */
        close(masterFd);

        errno = savedErrno;
        return -1;
    }

    return masterFd;
}
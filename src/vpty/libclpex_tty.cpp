#include "libclpex_tty.h"

#define MAX_SNAME 1000                  /* Maximum size for pty slave name */
struct termios ttyOrig;

/* Reset terminal mode on program exit */
static void ttyReset(void)
{
    if (tcsetattr(STDIN_FILENO, TCSANOW, &ttyOrig) == -1)
        perror("tcsetattr");
}

pid_t ptyFork(int *masterFd, char *slaveName, size_t snLen,
        const struct termios *slaveTermios, const struct winsize *slaveWS)
{
    int mfd, slaveFd, savedErrno;
    pid_t childPid;
    char slname[MAX_SNAME];

    mfd = ptyMasterOpen(slname, MAX_SNAME);
    if (mfd == -1)
        return -1;

    if (slaveName != NULL) {            /* Return slave name to caller */
        if (strlen(slname) < snLen) {
            strncpy(slaveName, slname, snLen);

        } else {                        /* 'slaveName' was too small */
            close(mfd);
            errno = EOVERFLOW;
            return -1;
        }
    }

    childPid = fork();

    if (childPid == -1) {               /* fork() failed */
        savedErrno = errno;             /* close() might change 'errno' */
        close(mfd);                     /* Don't leak file descriptors */
        errno = savedErrno;
        return -1;
    }

    if (childPid != 0) {                /* Parent */
        *masterFd = mfd;                /* Only parent gets master fd */
        return childPid;                /* Like parent of fork() */
    }

    /* Child falls through to here */

    if (setsid() == -1)                 /* Start a new session */
        perror("ptyFork:setsid");

    close(mfd);                         /* Not needed in child */

    slaveFd = open(slname, O_RDWR);     /* Becomes controlling tty */
    if (slaveFd == -1)
        perror("ptyFork:open-slave");

#ifdef TIOCSCTTY                        /* Acquire controlling tty on BSD */
    if (ioctl(slaveFd, TIOCSCTTY, 0) == -1)
        perror("ptyFork:ioctl-TIOCSCTTY");
#endif

    if (slaveTermios != NULL)           /* Set slave tty attributes */
        if (tcsetattr(slaveFd, TCSANOW, slaveTermios) == -1)
            perror("ptyFork:tcsetattr");

    if (slaveWS != NULL)                /* Set slave tty window size */
        if (ioctl(slaveFd, TIOCSWINSZ, slaveWS) == -1)
            perror("ptyFork:ioctl-TIOCSWINSZ");

    /* Duplicate pty slave to be child's stdin, stdout, and stderr */

    if (dup2(slaveFd, STDIN_FILENO) != STDIN_FILENO)
        perror("ptyFork:dup2-STDIN_FILENO");
    if (dup2(slaveFd, STDOUT_FILENO) != STDOUT_FILENO)
        perror("ptyFork:dup2-STDOUT_FILENO");
    if (dup2(slaveFd, STDERR_FILENO) != STDERR_FILENO)
        perror("ptyFork:dup2-STDERR_FILENO");

    if (slaveFd > STDERR_FILENO)        /* Safety check */
        close(slaveFd);                 /* No longer need this fd */

    return 0;                           /* Like child of fork() */
}

// this does three things:
//  - execute a dang string as a subprocess command
//  - capture child stdout/stderr to respective log files
//  - TEE child stdout/stderr to parent stdout/stderr
int execute_pty( std::string command, std::string stdout_log_file, std::string stderr_log_file )
{
    // turn our command string into something execvp can consume
    char ** processed_command = expand_env(command );

    // open file handles to the two log files we need to create for each execution
    FILE * stdout_log_fh = fopen( stdout_log_file.c_str(), "a+" );
    FILE * stderr_log_fh = fopen( stderr_log_file.c_str(), "a+" );

    // create the pipes for the child process to write and read from using its stdin/stdout/stderr
    int fd_child_stdout_pipe[2];
    int fd_child_stderr_pipe[2];

    if ( pipe(fd_child_stdout_pipe ) == -1 ) {
        perror( "child stdout pipe" );
        exit( 1 );
    }

    if ( pipe( fd_child_stderr_pipe ) == -1 ) {
        perror( "child stderr pipe" );
        exit( 1 );
    }

    // using O_CLOEXEC to ensure that the child process closes the file descriptors
    if ( fcntl( fd_child_stdout_pipe[READ_END], F_SETFD, FD_CLOEXEC ) == -1 ) {
        perror("fcntl");
        exit(1);
    }
    // same for stderr
    if ( fcntl( fd_child_stderr_pipe[READ_END], F_SETFD, FD_CLOEXEC ) == -1 ) {
        perror("fcntl");
        exit(1);
    }

    // same for stdout write
    if ( fcntl( fd_child_stdout_pipe[WRITE_END], F_SETFD, FD_CLOEXEC ) == -1 ) {
        perror("fcntl");
        exit(1);
    }

    //    // same for stderr write
    if ( fcntl( fd_child_stderr_pipe[WRITE_END], F_SETFD, FD_CLOEXEC ) == -1 ) {
        perror("fcntl");
        exit(1);
    }

    // status result basket for the parent process to capture the child's exit status
    int status = 616;

    // start ptyfork integration
    char slaveName[MAX_SNAME];
    char *shell;
    int masterFd, scriptFd;
    struct winsize ws;
    ssize_t numRead;


    /* Retrieve the attributes of terminal on which we are started */
    if (tcgetattr(STDIN_FILENO, &ttyOrig) == -1)
        perror("tcgetattr");
    if (ioctl(STDIN_FILENO, TIOCGWINSZ, &ws) < 0)
        perror("ioctl-TIOCGWINSZ");

    pid_t pid = ptyFork( &masterFd, slaveName, MAX_SNAME, &ttyOrig, &ws );

    switch( pid ) {
        case -1:
        {
            // fork failed
            perror("ptyfork failure");
            exit(1);
        }

        case 0:
        {
            // child process

            // close the file descriptor STDOUT_FILENO if it was previously open, then (re)open it as a copy of
            //while ((dup2(fd_child_stdout_pipe[WRITE_END], STDOUT_FILENO) == -1) && (errno == EINTR)) {}
            while ((dup2(fd_child_stderr_pipe[WRITE_END], STDERR_FILENO) == -1) && (errno == EINTR)) {}

            // execute the dang command, print to stdout, stderr (of parent), and dump to file for each!!!!
            // (and capture exit code in parent)
            int exit_code = execvp(processed_command[0], processed_command);
            perror("failed on execvp in child");
            exit(exit_code);
        }

        default:
        {
            // parent process

            // start ptyfork integration
            ttySetRaw(STDIN_FILENO, &ttyOrig);

            if (atexit(ttyReset) != 0)
                perror("atexit");




            // The parent process has no need to access the entrance to the pipe, so fd_child_*_pipe[1|0] should be closed
            // within that process too:
            close(fd_child_stdout_pipe[WRITE_END]);
            close(fd_child_stderr_pipe[WRITE_END]);

            // attempt to write to stdout,stderr from child as well as to write each to file
            char buf[BUFFER_SIZE];

            // contains the byte count of the last read from the pipe
            ssize_t byte_count;

            // watched_fds for poll() to wait on
            struct pollfd watched_fds[3];

            // populate the watched_fds array

            // child STDOUT to parent
            watched_fds[0].fd = STDIN_FILENO;
            watched_fds[0].events = POLLIN;

            // child STDERR to parent
            watched_fds[1].fd = masterFd;
            watched_fds[1].events = POLLIN;

            watched_fds[2].fd = fd_child_stderr_pipe[READ_END];
            watched_fds[2].events = POLLIN;

            // number of files poll() reports as ready
            int num_files_readable;

            // loop flag
            bool break_out = false;

            // loop until we've read all the data from the child process
            while ( ! break_out ) {
                num_files_readable = poll(watched_fds, sizeof(watched_fds) / sizeof(watched_fds[0]), -1);
                if (num_files_readable == -1) {
                    // error occurred in poll()
                    perror("poll");
                    exit(1);
                }
                if (num_files_readable == 0) {
                    // poll reports no files readable
                    break_out = true;
                    break;
                }
                for (int this_fd = 0; this_fd < 3; this_fd++) {
                    if (watched_fds[this_fd].revents & POLLIN) {
                        // this pipe is readable
                        byte_count = read(watched_fds[this_fd].fd, buf, BUFFER_SIZE);

                        if (byte_count == -1) {
                            // error reading from pipe
                            perror("read");
                            exit(EXIT_FAILURE);
                        } else if (byte_count == 0) {
                            // reached EOF on one of the streams but not a HUP
                            // we've read all we can this cycle, so go to the next fd in the for loop
                            continue;
                            //break;
                        } else {
                            // byte count was sane
                            // write to stdout,stderr
                            if (this_fd == 0) {
                                // the child's stdout pipe is readable
                                write(masterFd, buf, byte_count);
                            } else if (this_fd == 1 ) {
                                // the child's stdout pipe is readable
                                write(stdout_log_fh->_fileno, buf, byte_count);
                                write(STDOUT_FILENO, buf, byte_count);
                            } else if ( this_fd == 2 ){
                                //the child's stderr
                                write(stderr_log_fh->_fileno, buf, byte_count);
                                write(STDERR_FILENO, buf, byte_count);
                            } else {
                                // this should never happen
                                perror("Logic error!");
                                exit(EXIT_FAILURE);
                            }
                        }
                    }
                    if (watched_fds[this_fd].revents & POLLERR) {
                        close(watched_fds[this_fd].fd);
                        break_out = true;
                    }
                    if (watched_fds[this_fd].revents & POLLHUP) {
                        // this pipe has hung up
                        // go to the next fd in the for loop
                        close(watched_fds[this_fd].fd);
                        break_out = true;
                        //break;
                    }

                }
            }
            // wait for child to exit, capture status
            waitpid(pid, &status, 0);

            // close the log file handles
            fclose(stdout_log_fh);
            fclose(stderr_log_fh);
            return status;
        }
    }
}
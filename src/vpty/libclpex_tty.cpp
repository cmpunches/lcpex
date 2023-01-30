#include "libclpex_tty.h"


int execute_pty( std::string command, std::string stdout_log_file, std::string stderr_log_file )
{
    // status result basket for the parent process to capture the child's exit status
    int status = 616;

    // turn our command string into something execvp can consume
    char ** processed_command = expand_env(command );

    const struct termios * slaveTermios;
    const  struct winsize * slaveWS;

    int mfd, slaveFd, savedErrno;
    char slname[20];

    // open master
    mfd = ptyMasterOpen();

    // if the new master vpty is invalid, return an error
    if ( mfd == -1 )
    {
        return -1;
    }

    // fork a child
    pid_t pid = fork();

    switch ( pid )
    {
        case -1:
        {
            /* close() might change 'errno' */
            savedErrno = errno;
            /* Don't leak file descriptors */
            close(mfd);
            errno = savedErrno;
            return -1;
        }

        case 0:
        {
            // child process
            /* Child falls through to here */

            /* Start a new session */
            if (setsid() == -1)
                perror("ptyFork:setsid");

            /* Not needed in child */
            close(mfd);

            /* Becomes controlling tty */
            slaveFd = open(slname, O_RDWR);
            if (slaveFd == -1)
                perror("ptyFork:open-slave");

            /* Set slave tty attributes */
            if (slaveTermios != nullptr)
                if (tcsetattr(slaveFd, TCSANOW, slaveTermios) == -1)
                    perror("ptyFork:tcsetattr");

            /* Set slave tty window size */
            if (slaveWS != nullptr)
                if (ioctl(slaveFd, TIOCSWINSZ, slaveWS) == -1)
                    perror("ptyFork:ioctl-TIOCSWINSZ");

            // Duplicate vpty slave to be child's stdin, stdout, and stderr
            if (dup2(slaveFd, STDIN_FILENO) != STDIN_FILENO)
                perror("ptyFork:dup2-STDIN_FILENO");
            if (dup2(slaveFd, STDOUT_FILENO) != STDOUT_FILENO)
                perror("ptyFork:dup2-STDOUT_FILENO");
            if (dup2(slaveFd, STDERR_FILENO) != STDERR_FILENO)
                perror("ptyFork:dup2-STDERR_FILENO");

            // Safety check
            if (slaveFd > STDERR_FILENO)
            {
                // No longer need this fd
                close(slaveFd);
            }

            /* begin original fork() code */
            // execute the dang command, print to stdout, stderr (of parent), and dump to file for each!!!!
            // (and capture exit code in parent)
            int exit_code = execvp(processed_command[0], processed_command);
            perror("failed on execvp in child");
            exit(exit_code);
        }

        default:
        {
            // parent process
            // wait for child to exit, capture status
            waitpid(pid, &status, 0);

            // close the log file handles
            //fclose(stdout_log_fh);
            //fclose(stderr_log_fh);
            return status;
        }
    }

    // Like child of fork()
    return 0;
}


// this does three things:
//  - execute a dang string as a subprocess command
//  - capture child stdout/stderr to respective log files
//  - TEE child stdout/stderr to parent stdout/stderr
int execute_pty2( std::string command, std::string stdout_log_file, std::string stderr_log_file )
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
    int ptyFD = -1;
    pid_t pid = forkpty( &ptyFD, nullptr, nullptr, nullptr );

    switch( pid ) {
        case -1:
        {
            // fork failed
            perror("fork failure");
            exit(1);
        }

        case 0:
        {
            // child process

            // close the file descriptor STDOUT_FILENO if it was previously open, then (re)open it as a copy of
            while ((dup2(fd_child_stdout_pipe[WRITE_END], STDOUT_FILENO) == -1) && (errno == EINTR)) {}
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

            // The parent process has no need to access the entrance to the pipe, so fd_child_*_pipe[1|0] should be closed
            // within that process too:
            close(fd_child_stdout_pipe[WRITE_END]);
            close(fd_child_stderr_pipe[WRITE_END]);

            // attempt to write to stdout,stderr from child as well as to write each to file
            char buf[BUFFER_SIZE];

            // contains the byte count of the last read from the pipe
            ssize_t byte_count;

            // watched_fds for poll() to wait on
            struct pollfd watched_fds[2];

            // populate the watched_fds array

            // child STDOUT to parent
            watched_fds[CHILD_PIPE_NAMES::STDOUT_READ].fd = fd_child_stdout_pipe[READ_END];
            watched_fds[CHILD_PIPE_NAMES::STDOUT_READ].events = POLLIN;

            // child STDERR to parent
            watched_fds[CHILD_PIPE_NAMES::STDERR_READ].fd = fd_child_stderr_pipe[READ_END];
            watched_fds[CHILD_PIPE_NAMES::STDERR_READ].events = POLLIN;

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
                for (int this_fd = 0; this_fd < 2; this_fd++) {
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
                            if (this_fd == CHILD_PIPE_NAMES::STDOUT_READ) {
                                // the child's stdout pipe is readable
                                write(stdout_log_fh->_fileno, buf, byte_count);
                                write(STDOUT_FILENO, buf, byte_count);
                            } else if (this_fd == CHILD_PIPE_NAMES::STDERR_READ) {
                                // the child's stderr pipe is readable
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
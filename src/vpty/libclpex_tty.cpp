#include "libclpex_tty.h"

void safe_perror( const char * msg, struct termios * ttyOrig )
{
    std::cerr << msg << std::endl;
    ttyResetExit( ttyOrig );
    exit(1);
}

// this does three things:
//  - execute a dang string as a subprocess command
//  - capture child stdout/stderr to respective log files
//  - TEE child stdout/stderr to parent stdout/stderr
int execute_pty( std::string command, std::string stdout_log_file, std::string stderr_log_file )
{
    struct termios ttyOrig;

    // turn our command string into something execvp can consume
    char ** processed_command = expand_env(command );

    // open file handles to the two log files we need to create for each execution
    FILE * stdout_log_fh = fopen( stdout_log_file.c_str(), "a+" );
    FILE * stderr_log_fh = fopen( stderr_log_file.c_str(), "a+" );

    // create the pipes for the child process to write and read from using its stderr
    int fd_child_stderr_pipe[2];

    if ( pipe( fd_child_stderr_pipe ) == -1 ) {
        safe_perror( "child stderr pipe", &ttyOrig );
        exit( 1 );
    }

    // using O_CLOEXEC to ensure that the child process closes the file descriptors
    if ( fcntl( fd_child_stderr_pipe[READ_END], F_SETFD, FD_CLOEXEC ) == -1 ) { perror("fcntl"); exit(1); }

    //    // same for stderr write
    if ( fcntl( fd_child_stderr_pipe[WRITE_END], F_SETFD, FD_CLOEXEC ) == -1 ) { perror("fcntl"); exit(1); }

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
        safe_perror("tcgetattr", &ttyOrig);
    if (ioctl(STDIN_FILENO, TIOCGWINSZ, &ws) < 0)
        safe_perror("ioctl-TIOCGWINSZ", &ttyOrig );

    pid_t pid = ptyFork( &masterFd, slaveName, MAX_SNAME, &ttyOrig, &ws );

    switch( pid ) {
        case -1:
        {
            // fork failed
            safe_perror("ptyfork failure", &ttyOrig );
            exit(1);
        }

        case 0:
        {
            // child process

            // close the file descriptor STDERR_FILENO if it was previously open, then (re)open it as a copy of
            while ((dup2(fd_child_stderr_pipe[WRITE_END], STDERR_FILENO) == -1) && (errno == EINTR)) {}

            // execute the dang command, print to stdout, stderr (of parent), and dump to file for each!!!!
            // (and capture exit code in parent)
            int exit_code = execvp(processed_command[0], processed_command);
            safe_perror("failed on execvp in child", &ttyOrig );
            exit(exit_code);
        }

        default:
        {
            // parent process

            // start ptyfork integration
            ttySetRaw(STDIN_FILENO, &ttyOrig);

            // The parent process has no need to access the entrance to the pipe, so fd_child_*_pipe[1|0] should be
            // closed within that process too:
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
                    safe_perror("poll", &ttyOrig );
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
                            safe_perror("read", &ttyOrig );
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
            ttyResetExit( &ttyOrig);
            return WEXITSTATUS( status );
        }
    }
}
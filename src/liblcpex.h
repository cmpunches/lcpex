#ifndef LCPEX_LIBLCPEX_H
#define LCPEX_LIBLCPEX_H

#include <cstdio>
#include <cstdlib>
#include <poll.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/wait.h>
#include <iostream>

#define BUFFER_SIZE 1024

// helper for sanity
enum PIPE_ENDS {
    READ_END = 0,
    WRITE_END = 1
};

// helper for sanity
// these do NOT represent fd values
enum CHILD_PIPE_NAMES {
    STDOUT_READ = 0,
    STDERR_READ = 1
};

// execute
// uses fork/pipe/dup2/execvp using only pipes for i/o stream redirection
// this will tee output to the parent stdout/stderr in addition to respective
// log paths specified as arguments
int execute( std::string command, std::string stdout_log_file, std::string stderr_log_file );

// execute_pty
// same as execute, using extraction from pty_fork
// creates a slave pty for child spawning and routes i/o in the same manner between parent
// and slave
// ref: https://man7.org/tlpi/code/online/dist/pty/pty_fork.c.html
int execute_pty( std::string command, std::string stdout_log_file, std::string stderr_log_file );

#endif //LCPEX_LIBLCPEX_H

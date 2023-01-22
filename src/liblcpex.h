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

int execute( std::string command, std::string stdout_log_file, std::string stderr_log_file );



#endif //LCPEX_LIBLCPEX_H

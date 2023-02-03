#ifndef LCPEX_LIBLCPEX_H
#define LCPEX_LIBLCPEX_H

#include <cstdio>
#include <cstdlib>
#include <poll.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/wait.h>
#include <iostream>
#include "string_expansion/string_expansion.h"
#include "helpers.h"

// execute
// uses fork/pipe/dup2/execvp using only pipes for i/o stream redirection
// this will tee output to the parent stdout/stderr in addition to respective
// log paths specified as arguments
int execute( std::string command, std::string stdout_log_file, std::string stderr_log_file );



#endif //LCPEX_LIBLCPEX_H

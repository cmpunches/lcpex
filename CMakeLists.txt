cmake_minimum_required(VERSION 3.23)
project(lcpex)

set(CMAKE_CXX_STANDARD 14)

add_executable(lcpex lcpex.cpp src/string_expansion.cpp src/string_expansion.h src/liblcpex.cpp src/liblcpex.h src/vpty/pty_fork_mod/pty_master_open.cpp src/vpty/pty_fork_mod/pty_master_open.h src/vpty/libclpex_tty.cpp src/vpty/libclpex_tty.h src/helpers.h src/vpty/pty_fork_mod/tty_functions.cpp src/vpty/pty_fork_mod/tty_functions.h)

cmake_minimum_required(VERSION 3.23)
project(lcpex)

set(CMAKE_CXX_STANDARD 14)

add_executable(lcpex lcpex.cpp src/string_expansion.cpp src/string_expansion.h src/liblcpex.cpp src/liblcpex.h)

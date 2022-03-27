cmake_minimum_required(VERSION 3.20)
project(ex2 C)

set(CMAKE_C_STANDARD 11)

add_executable(ex2 server.c client.c)

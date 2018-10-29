cmake_minimum_required(VERSION 3.12)
project(Task1 C)

set(CMAKE_C_STANDARD 90)

add_executable(Task1 main.c id-list.c cmake-build-debug/token-list.c token-list.h)
cmake_minimum_required(VERSION 3.10)
project(Lab_7 C)

set(CMAKE_C_STANDARD 11)

include_directories(.)

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

add_executable(Lab_7 networking.c networking.h node.c linkedlist.c)

target_link_libraries(Lab_7 Threads::Threads)

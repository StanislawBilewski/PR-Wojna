cmake_minimum_required(VERSION 3.21)
project(PR_Wojna)

set(CMAKE_CXX_STANDARD 14)

include_directories(.)

add_executable(PR_Wojna
        comm_thread.cpp
        comm_thread.h
        constants.h
        main.cpp
        main.h
        main_thread.cpp
        main_thread.h
        structures.cpp
        structures.h)

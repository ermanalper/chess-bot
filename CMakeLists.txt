cmake_minimum_required(VERSION 4.0)
project(chess_bot)

set(CMAKE_CXX_STANDARD 20)

add_executable(chess_bot main.cpp
        functions.c
        listnode.h
        hash.c
        hash.h)

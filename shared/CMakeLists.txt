cmake_minimum_required(VERSION 3.22)
project(shared)


add_library(shared STATIC 
            message_struct.h 
            message_struct.cpp
            filedata.h
            filedata.cpp)
target_include_directories(shared PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
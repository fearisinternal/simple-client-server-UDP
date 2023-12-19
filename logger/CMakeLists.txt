cmake_minimum_required(VERSION 3.22)
project(logger)

add_library(logger STATIC
            logger.h
            logger.cpp)
target_include_directories(logger PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
cmake_minimum_required(VERSION 3.22)
project(shared)

add_library(shared STATIC message_struct.h message_struct.cpp)
cmake_minimum_required(VERSION 3.22)
project(server)

add_executable(server server.cpp)
target_link_libraries(server PRIVATE logger)
target_link_libraries(server PRIVATE shared)
cmake_minimum_required(VERSION 3.5)
project(udp_file_sharing_server)
add_executable(udp_file_sharing_server server.cpp base_struct.h)

project(udp_file_sharing_client)
add_executable(udp_file_sharing_client client.cpp base_struct.h)
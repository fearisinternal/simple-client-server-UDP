#pragma once
#ifndef FILEDATA_H
#define FILEDATA_H

#include "message_struct.h"

#include <iostream>
#include <algorithm>
#include <fstream>
#include <cstring>
#include <ctime>
#include <vector>
#include <set>

static constexpr size_t BUFFER_ANSWER_SIZE = sizeof(UDP_MessageHeader) + sizeof(uint32_t);

class FileData
{
public:
    bool open_file(const char *filename);

    bool save_from_message(UDP_MessageHeader message,
                           std::array<std::byte, MAX_MESSAGE_SIZE> received_buffer,
                           ssize_t bytes_count);

    std::array<std::byte, BUFFER_ANSWER_SIZE> create_server_message(UDP_MessageHeader message);

    uint64_t create_client_message(std::array<std::byte, MAX_LINE_SIZE> &send_buffer);

    MessageId id;
    std::vector<char> file_data;
    std::set<uint32_t> parts;
    uint64_t file_size;
    uint32_t parts_count;
};

#endif // FILEDATA_H
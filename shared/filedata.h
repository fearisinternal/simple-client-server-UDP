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

    bool save_data_from_message(const UDP_MessageHeader &message,
                                const std::array<std::byte, MAX_MESSAGE_SIZE> &received_buffer,
                                const ssize_t &bytes_count);
    
    /// @brief Create server message
    /// return - buffer message
    std::array<std::byte, BUFFER_ANSWER_SIZE> create_server_message(UDP_MessageHeader &message);
    
    /// @brief Create client message
    /// return - size of buffer message 
    uint64_t create_client_message(std::array<std::byte, MAX_LINE_SIZE> &send_buffer);

private:
    MessageId id;

public:
    uint32_t parts_total;
    uint64_t file_size;
    std::set<uint32_t> parts;
    std::vector<char> file_data;
};

#endif // FILEDATA_H
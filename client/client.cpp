#include <iostream>
#include <fstream>
#include <algorithm>

#include <netinet/in.h>
#include <netinet/ip.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>

#include <cstring>
#include <vector>
#include <set>

#include "message_struct.h"

int main(int argc, char *argv[])
{
    std::cout << "Client is running" << std::endl;
    if (argc < 2)
    {
        std::cout << "Add path to file!" << std::endl;
    }

    auto udp_socket_client = start_socket();

    auto server_addr = get_address(udp_socket_client);

    // todo -- file from console
    const char *filename = argv[1];

    std::ifstream file(filename, std::ios::binary);
    auto file_end = file.seekg(0, std::ios::end).tellg();
    auto file_size = file_end - file.seekg(0, std::ios::beg).tellg();
    std::vector<char> file_data(file_size);
    file.read(file_data.data(), file_data.size());

    auto parts_count = (file_size + MAX_LINE_SIZE - 1) / MAX_LINE_SIZE;
    std::set<uint32_t> unsent_parts;
    for (uint32_t i = 0; i < parts_count; i++)
    {
        unsent_parts.insert(i);
    }

    MessageId message_id;
    for (auto i = 0; i < ID_LENGTH; i++)
    {
        message_id[i] = rand() & 0x00ff;
    }
    uint32_t crc_get = 0;
    while (!unsent_parts.empty())
    {
        {
            auto part = std::begin(unsent_parts);
            std::advance(part, rand() % unsent_parts.size());
            std::cout << "Send part " << *part << std::endl;

            auto first_point = *part * MAX_LINE_SIZE;
            auto last_point = std::min((uint64_t)(first_point + MAX_LINE_SIZE), (uint64_t)file_size);
            auto message_size = last_point - first_point;
            std::string bytes = std::string(&file_data[first_point], &file_data[last_point]);

            UDP_MessageHeader message_to_send{*part, static_cast<uint32_t>(parts_count),
                                              UDP_MessageHeader::Type::PUT, message_id};

            unsigned char message_data[bytes.size()];
            strcpy((char *)message_data, bytes.c_str());

            std::array<std::byte, MAX_LINE_SIZE> send_buffer = {};
            memcpy(send_buffer.data(), &message_to_send, sizeof(message_to_send));
            memcpy(send_buffer.data() + sizeof(message_to_send), file_data.data() + first_point, message_size);

            sendto(udp_socket_client, static_cast<void *>(&send_buffer),
                   sizeof(UDP_MessageHeader) + message_size, 0,
                   reinterpret_cast<sockaddr *>(&server_addr), sizeof(server_addr));
        }
        std::array<std::byte, MAX_MESSAGE_SIZE> received_buffer;
        socklen_t server_lenght = sizeof(server_addr);
        auto bytes_in = recvfrom(udp_socket_client, received_buffer.data(),
                                 MAX_MESSAGE_SIZE, 0,
                                 reinterpret_cast<sockaddr *>(&server_addr),
                                 &server_lenght);

        /// TODO: check result

        UDP_MessageHeader get_message = {};
        memcpy(&get_message, received_buffer.data(), sizeof(UDP_MessageHeader));
        size_t message_size = bytes_in - sizeof(UDP_MessageHeader);

        if (message_size > 0)
        {
            memcpy(&crc_get, received_buffer.data() + sizeof(UDP_MessageHeader), message_size);
        }

        unsent_parts.erase(get_message.seq_number);
    }

    uint32_t crc = crc32c(0, reinterpret_cast<unsigned char *>(file_data.data()), file_data.size());
    std::cout << "crc_client: " << crc << std::endl;
    std::cout << "crc_server: " << crc_get << std::endl;

    return 0;
}
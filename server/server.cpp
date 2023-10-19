#include <iostream>
#include <fstream>

#include <arpa/inet.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>

#include <cstring>
#include <vector>
#include <set>

#include "message_struct.h"

int main()
{
    std::cout << "Server was run" << std::endl;

    auto udp_socket_server = socket(AF_INET, SOCK_DGRAM, 0);

    if (udp_socket_server < 0)
    {
        std::cerr << "\nError socket creation..." << std::endl;
        return 0;
    }

    // Server information
    sockaddr_in server_addr = {};
    server_addr.sin_family = AF_INET; // IPv4
    server_addr.sin_port = htons(PORT);
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);

    if (bind(udp_socket_server, (sockaddr *)&server_addr, sizeof(server_addr)) == -1)
    {
        std::cerr << "Binding was failed..." << std::endl;
        return 0;
    }
    std::vector<std::byte> file_data; //todo std::byte
    std::set<uint32_t> received_parts;
    while (true)
    {
        std::array<std::byte, MAX_MESSAGE_SIZE> received_buffer;
        sockaddr_in client_addr = {};
        socklen_t client_length = sizeof(client_addr);
        auto bytes_in = recvfrom(udp_socket_server, received_buffer.data(),
                                    MAX_MESSAGE_SIZE, 0,
                                    reinterpret_cast<sockaddr *>(&client_addr),
                                    &client_length);

        //todo check correct received message -- test & -1

        UDP_MessageHeader message;
        memcpy(&message, received_buffer.data(), sizeof(UDP_MessageHeader));

        received_parts.insert(message.seq_number);
        auto start = message.seq_number * MAX_LINE_SIZE;
        size_t message_size = bytes_in - sizeof(UDP_MessageHeader);
        if (start + message_size > file_data.size())
        {
            file_data.resize(start + message_size);
        }

        memcpy(file_data.data() + start, received_buffer.data() + sizeof(UDP_MessageHeader), message_size);

        message.type = UDP_MessageHeader::Type::ACK;

        if (received_parts.size() == message.seq_total)
        {
            auto crc = crc32c(0, reinterpret_cast<unsigned char *>(file_data.data()), file_data.size());

            char buffer_send[sizeof(message) + sizeof(crc)];
            memcpy(&buffer_send, &message, sizeof(message));
            memcpy(&buffer_send[sizeof(message)], &crc, sizeof(crc));
            sendto(udp_socket_server, static_cast<void*>(&buffer_send), 
                    sizeof(buffer_send), 0,
                   reinterpret_cast<sockaddr*>(&client_addr),
                   sizeof(client_addr));
            break;
        }
        char buffer_send[sizeof(message)];
        memcpy(&buffer_send, &message, sizeof(message));
        sendto(udp_socket_server, static_cast<void*>(&buffer_send), 
                sizeof(buffer_send), 0,
                reinterpret_cast<sockaddr*>(&client_addr),
                sizeof(client_addr));
    }

    close(udp_socket_server);

    return 0;
}
#include <iostream>
#include <fstream>

#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <unistd.h>
#include <string.h>

#include <vector>
#include <set>

#include "base_struct.h"

int main()
{
    std::cout << "Server was run" << std::endl;

    auto udp_socket_server = socket(AF_INET, SOCK_DGRAM, 0); // IPPROTO_UDP

    if (udp_socket_server < 0)
    {
        std::cout << "\nError socket creation..." << std::endl;
        return 0;
    }

    sockaddr_in server_addr, client_addr;
    socklen_t clientLenght = sizeof(client_addr);

    // Server information
    server_addr.sin_family = AF_INET; // IPv4
    server_addr.sin_port = htons(PORT);
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);

    if (bind(udp_socket_server, (sockaddr *)&server_addr, sizeof(server_addr)) == -1)
    {
        std::cout << "Binding was failed..." << std::endl;
        return 0;
    }
    std::vector<char> file_data;
    std::set<uint32_t> get_parts;
    while (1)
    {
        char buffer_get[MAX_SIZE];
        ssize_t bytes_in = recvfrom(udp_socket_server, (char *)buffer_get,
                                    MAX_SIZE, MSG_WAITALL,
                                    reinterpret_cast<sockaddr *>(&client_addr),
                                    &clientLenght);
        UDP_Message message;
        memcpy((char *)&message, buffer_get, sizeof(UDP_Message));
        
        std::cout << "GET " << message.seq_number << std::endl;

        get_parts.insert(message.seq_number);
        int64_t start = (message.seq_number - 1) * (MAX_LINE);
        size_t size_message = bytes_in - sizeof(UDP_Message);
        if (start + size_message > file_data.size())
        {
            file_data.resize(start + size_message);
        }

        memcpy((char *)&file_data[start], buffer_get + sizeof(UDP_Message), size_message);

        message.type = 0;

        if (get_parts.size() == message.seq_total)
        {
            uint32_t crc = crc32c(0, reinterpret_cast<unsigned char *>(file_data.data()), file_data.size());

            char buffer_send[sizeof(message) + sizeof(crc)];
            memcpy(&buffer_send, &message, sizeof(message));
            memcpy(&buffer_send[sizeof(message)], &crc, sizeof(crc));
            sendto(udp_socket_server, (void *)&buffer_send, sizeof(buffer_send),
                   MSG_CONFIRM, (const struct sockaddr *)&client_addr,
                   sizeof(client_addr));
            break;
        }
        char buffer_send[sizeof(message)];
        memcpy(&buffer_send, &message, sizeof(message));
        sendto(udp_socket_server, (void *)&buffer_send, sizeof(buffer_send),
               MSG_CONFIRM, (const struct sockaddr *)&client_addr,
               sizeof(client_addr));
    }

    close(udp_socket_server);

    return 0;
}
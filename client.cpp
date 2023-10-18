#include <iostream>
#include <fstream>
#include <algorithm>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <string.h>
#include <unistd.h>

#include <vector>
#include <set>

#include "base_struct.h"

int main()
{
    std::cout << "Client was run" << std::endl;

    auto udp_socket_client = socket(AF_INET, SOCK_DGRAM, 0);

    if (udp_socket_client < 0)
    {
        std::cout << "\nError socket creation..." << std::endl;
        return 0;
    }

    sockaddr_in server_addr, client_addr;
    socklen_t serverLenght = sizeof(server_addr);

    // Server information
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);

    // File transfer

    // const char *filename = "input.txt";
    //const char *filename = "test_text1.txt";
    // const char * filename = "test_pic.png";
    const char *filename = "test_text2.txt";

    std::ifstream file{filename, std::ios::binary};
    auto end{file.seekg(0, std::ios::end).tellg()};
    auto file_size{end - file.seekg(0, std::ios::beg).tellg()};
    std::vector<char> file_data(file_size);
    file.read(file_data.data(), file_data.size());

    uint32_t total_size = (file_size / (MAX_LINE)) + 1;
    std::set<uint32_t> unsentedParts; // нумерация неотправленных частей
    for (uint32_t i = 1; i < total_size + 1; i++)
    {
        unsentedParts.insert(i);
    }

    unsigned char message_id[ID_LENGTH];
    for (auto i = 0; i < ID_LENGTH; i++)
    {
        message_id[i] = (rand() & 0x00ff);
    }
    uint32_t crc_get = 0;
    while (!unsentedParts.empty())
    {
        auto part = std::begin(unsentedParts);
        std::advance(part, rand() % unsentedParts.size());

        int64_t start = (*part - 1) * (MAX_LINE);
        int64_t finish = std::min((uint64_t)(start + MAX_LINE), (uint64_t)(file_size));
        std::string bytes = std::string(&file_data[start], &file_data[finish]);

        unsigned char message_data[bytes.size()];
        strcpy((char *)message_data, bytes.c_str());

        UDP_Message message_to_send{*part, total_size, 1};
        memcpy(message_to_send.id, message_id, ID_LENGTH);
        // memcpy(m1.data, message_data, bytes.size());

        char buffer_send[sizeof(message_to_send) + sizeof(message_data)];
        memcpy(&buffer_send, &message_to_send, sizeof(message_to_send));
        memcpy(&buffer_send[sizeof(message_to_send)], bytes.c_str(), bytes.size());

        sendto(udp_socket_client, (void *)&buffer_send, sizeof(buffer_send),
               MSG_CONFIRM, (const struct sockaddr *)&server_addr,
               sizeof(server_addr));

        char buffer_get[MAX_SIZE];
        ssize_t bytes_in = recvfrom(udp_socket_client, buffer_get,
                                    MAX_SIZE, MSG_WAITALL,
                                    reinterpret_cast<sockaddr *>(&server_addr),
                                    &serverLenght);
        UDP_Message get_message;
        memcpy((char *)&get_message, buffer_get, sizeof(UDP_Message));
        size_t size_message = bytes_in - sizeof(UDP_Message);

        if (size_message>0) {
            memcpy((char *)&crc_get, buffer_get + sizeof(UDP_Message), size_message);
        }
        std::cout << "GET " << get_message.seq_number << std::endl;

        unsentedParts.erase(get_message.seq_number);
    }
    uint32_t crc = crc32c(0, reinterpret_cast<unsigned char *>(file_data.data()), file_data.size());
    std::cout << "crc_client: " << crc << std::endl;
    std::cout << "crc_server: " << crc_get << std::endl;



    return 0;
}
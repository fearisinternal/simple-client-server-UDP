#include "filedata.h"

int main(int argc, char *argv[])
{
    std::cout << "Client is running" << std::endl;
    if (argc < 2)
    {
        std::cout << "Add path to file!" << std::endl;
    }

    auto udp_socket_client = start_socket();

    auto server_addr = get_address(udp_socket_client);

    const char *filename = argv[1];
    FileData filedata;
    filedata.open_file(filename);

    uint32_t crc_get = 0;

    while (!filedata.parts.empty())
    {
        {
            std::array<std::byte, MAX_LINE_SIZE> send_buffer;
            auto message_size = filedata.create_client_message(send_buffer);

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

        filedata.parts.erase(get_message.seq_number);
    }

    uint32_t crc = crc32c(0, reinterpret_cast<unsigned char *>(filedata.file_data.data()), filedata.file_size);
    std::cout << "crc_client: " << crc << std::endl;
    std::cout << "crc_server: " << crc_get << std::endl;

    return 0;
}
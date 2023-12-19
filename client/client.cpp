#include "filedata.h"
#include "logger.h"

int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        Logger::Error("Add path to file!");
        return 0;
    }
    Logger::Info("Client is running...");
    auto udp_socket_client = start_socket();

    auto server_addr = get_address(udp_socket_client);

    const char *filename = argv[1];
    uint32_t crc = 0;
    uint32_t crc_get = 0;
    do
    {
        FileData filedata;
        if (!filedata.open_file(filename))
        {
            Logger::Error("Cannot open file: ");
            Logger::Error(filename);
            return 0;
        }
        Logger::Info("Transfer file: ");
        Logger::Info(filename);
        while (!filedata.parts.empty())
        {
            std::array<std::byte, MAX_LINE_SIZE> send_buffer;
            auto message_size = filedata.create_client_message(send_buffer);

            sendto(udp_socket_client, static_cast<void *>(&send_buffer),
                   sizeof(UDP_MessageHeader) + message_size, 0,
                   reinterpret_cast<sockaddr *>(&server_addr), sizeof(server_addr));

            std::array<std::byte, MAX_MESSAGE_SIZE> received_buffer;

            struct timeval tv
            {
                10, 0
            }; // socket waits 10 seconds
            setsockopt(udp_socket_client, SOL_SOCKET, SO_RCVTIMEO, (const char *)&tv, sizeof tv);

            socklen_t server_lenght = sizeof(server_addr);
            auto bytes_in = recvfrom(udp_socket_client, received_buffer.data(),
                                     MAX_MESSAGE_SIZE, 0,
                                     reinterpret_cast<sockaddr *>(&server_addr),
                                     &server_lenght);
            if (bytes_in < 0)
            {
                continue;
            }
            UDP_MessageHeader get_message = {};
            memcpy(&get_message, received_buffer.data(), sizeof(UDP_MessageHeader));

            if (get_message.seq_total == filedata.parts_total)
            {
                memcpy(&crc_get, received_buffer.data() + sizeof(UDP_MessageHeader), bytes_in - sizeof(UDP_MessageHeader));
            }

            Logger::Trace(("Get confirmation message:" + std::to_string(get_message.seq_number)).c_str());
            filedata.parts.erase(get_message.seq_number);
        }

        crc = crc32c(0, reinterpret_cast<unsigned char *>(filedata.file_data.data()), filedata.file_size);
        Logger::Error(("crc_client: " + std::to_string(crc)).c_str());
        Logger::Error(("crc_server: " + std::to_string(crc_get)).c_str());
        if (crc == crc_get)
        {
            Logger::Error("Task was completed successfully!");
        }
        else
        {
            Logger::Error("CRC did not match, sending error!");
        }
    } while (crc != crc_get);

    return 0;
}
#include <map>

#include "filedata.h"
#include "logger.h"

int main()
{
    Logger::Info("Server is running");
    auto udp_socket_server = start_socket();
    if (udp_socket_server < 0)
    {
        Logger::Error("Cannot create socket");
        return 0;
    }
    auto server_addr = get_address(udp_socket_server);
    if (bind(udp_socket_server, (sockaddr *)&server_addr, sizeof(server_addr)) == -1)
    {
        Logger::Error("Binding was failed...");
        return 0;
    }

    // File Database
    std::map<MessageId, FileData> file_db;

    while (true)
    {
        std::array<std::byte, MAX_MESSAGE_SIZE> received_buffer;
        sockaddr_in client_addr = {};
        socklen_t client_length = sizeof(client_addr);
        auto bytes_in = recvfrom(udp_socket_server, received_buffer.data(),
                                 MAX_MESSAGE_SIZE, 0,
                                 reinterpret_cast<sockaddr *>(&client_addr),
                                 &client_length);

        UDP_MessageHeader message;
        memcpy(&message, received_buffer.data(), sizeof(UDP_MessageHeader));
        file_db[message.id].save_data_from_message(message, received_buffer, bytes_in);

        auto buffer_send = file_db[message.id].create_server_message(message);
        if (file_db[message.id].parts.size() == file_db[message.id].parts_total)
        {
            file_db.erase(message.id);
            Logger::Info("File was saved to server");
        }
        sendto(udp_socket_server, static_cast<void *>(&buffer_send),
               sizeof(buffer_send), 0,
               reinterpret_cast<sockaddr *>(&client_addr),
               sizeof(client_addr));
        Logger::Trace("Confirmation message sent");
    }

    close(udp_socket_server);

    return 0;
}
#include <map>

#include "filedata.h"

int main()
{
    std::cout << "Server is running" << std::endl;
    auto udp_socket_server = start_socket();
    if (udp_socket_server < 0)
    {
        return 0;
    }
    auto server_addr = get_address(udp_socket_server);
    if (bind(udp_socket_server, (sockaddr *)&server_addr, sizeof(server_addr)) == -1)
    {
        std::cerr << "Binding was failed..." << std::endl;
        return 0;
    }
    
    //File Database
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
        message.seq_total = file_db[message.id].parts.size();
        message.type = UDP_MessageHeader::Type::ACK;

        auto buffer_send = file_db[message.id].create_server_message(message);

        sendto(udp_socket_server, static_cast<void *>(&buffer_send),
               sizeof(buffer_send), 0,
               reinterpret_cast<sockaddr *>(&client_addr),
               sizeof(client_addr));
    }

    close(udp_socket_server);
    
    return 0;
}
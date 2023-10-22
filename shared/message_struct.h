#pragma once

#include <cstddef>
#include <cstdint>
#include <array>
#include <iostream>

#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <unistd.h>

static constexpr size_t MAX_MESSAGE_SIZE = 1472;
static constexpr size_t ID_LENGTH = 8;
static constexpr size_t PORT = 1234;

using MessageId = std::array<uint8_t, ID_LENGTH>;

/// @brief Struct of message's package
#pragma pack(push, 1)
struct UDP_MessageHeader
{
    enum class Type : uint8_t
    {
        ACK = 0,
        PUT = 1
    };

    uint32_t seq_number; // номер пакета
    uint32_t seq_total;  // количество пакетов с данными
    Type type;
    MessageId id;
};
#pragma pack(pop)

static constexpr size_t MAX_LINE_SIZE = MAX_MESSAGE_SIZE - sizeof(UDP_MessageHeader);

/// @brief Function for calculating the checksum of received data
uint32_t crc32c(uint32_t crc, const unsigned char *buf, size_t len)
{
    crc = ~crc;
    while (len--)
    {
        crc ^= *buf++;
        for (auto k = 0; k < 8; k++)
            crc = crc & 1 ? (crc >> 1) ^ 0x82f63b78 : crc >> 1;
    }
    return ~crc;
}

sockaddr_in get_address(int udp_socket)
{
    sockaddr_in addr = {};
    addr.sin_family = AF_INET; // IPv4
    addr.sin_port = htons(PORT);
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    return addr;
}

int start_socket()
{
    auto udp_socket = socket(AF_INET, SOCK_DGRAM, 0);

    if (udp_socket < 0)
    {
        std::cerr << "\nError socket creation..." << std::endl;
        return -1;
    }
    return udp_socket;
}
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

/// @brief Struct of message's header
#pragma pack(push, 1)
struct UDP_MessageHeader
{
    enum class Type : uint8_t
    {
        ACK = 0,
        PUT = 1
    };

    uint32_t seq_number; // number of package
    uint32_t seq_total;  // total number of packages
    Type type;
    MessageId id;
};
#pragma pack(pop)

static constexpr size_t MAX_LINE_SIZE = MAX_MESSAGE_SIZE - sizeof(UDP_MessageHeader);

/// @brief Function for calculating the checksum of received data
extern uint32_t crc32c(uint32_t crc, const unsigned char *buf, size_t len);

/// @brief Create Internet socket address
extern sockaddr_in get_address(int udp_socket);

/// @brief Create a new socket
extern int start_socket();
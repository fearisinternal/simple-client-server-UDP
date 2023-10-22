#include "message_struct.h"

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
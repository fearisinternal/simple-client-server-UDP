#include <stddef.h>
#include <stdint.h>

static constexpr size_t MAX_SIZE = 1472;
static constexpr size_t ID_LENGTH = 8;
static constexpr size_t PORT = 1234;

#pragma pack(1)

/// @brief Struct of message's package
struct UDP_Message
{
    enum class Type : uint8_t {
        ACK = 0,
        PUT = 1
    };

    uint32_t seq_number;          // номер пакета
    uint32_t seq_total;           // количество пакетов с данными
    uint8_t type;                 // тип пакета: 0 == ACK, 1 == PUT
    unsigned char id[ID_LENGTH];  // 8 байт - идентификатор, отличающий один файл от другого
};

static constexpr size_t MAX_LINE = MAX_SIZE - sizeof(UDP_Message);

/// @brief Function for calculating the checksum of received data
uint32_t crc32c(uint32_t crc, const unsigned char *buf, size_t len)
{
    int k;
    crc = ~crc;
    while (len--)
    {
        crc ^= *buf++;
        for (k = 0; k < 8; k++)
            crc = crc & 1 ? (crc >> 1) ^ 0x82f63b78 : crc >> 1;
    }
    return ~crc;
}
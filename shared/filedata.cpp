#include "filedata.h"

bool FileData::open_file(const char *filename)
{
    std::ifstream file(filename, std::ios::binary);
    if (!file.is_open())
    {
        std::cerr << "File ile cannot be opened or does not exist" << std::endl;
        return false;
    }
    auto file_end = file.seekg(0, std::ios::end).tellg();
    file_size = file_end - file.seekg(0, std::ios::beg).tellg();
    file_data.resize(file_size);
    file.read(file_data.data(), file_data.size());

    parts_total = (file_size + MAX_LINE_SIZE - 1) / MAX_LINE_SIZE;
    for (uint32_t i = 0; i < parts_total; i++)
    {
        parts.insert(i);
    }
    srand(time(0));
    for (auto i = 0; i < ID_LENGTH; i++)
    {
        id[i] = rand() & 0x00ff;
    }
    return true;
}

bool FileData::save_data_from_message(UDP_MessageHeader message,
                                 std::array<std::byte, MAX_MESSAGE_SIZE> received_buffer,
                                 ssize_t bytes_count)
{
    parts.insert(message.seq_number);
    parts_total = message.seq_total;
    auto start = message.seq_number * MAX_LINE_SIZE;
    size_t message_size = bytes_count - sizeof(UDP_MessageHeader);
    if (start + message_size > file_data.size())
    {
        file_data.resize(start + message_size);
    }
    memcpy(file_data.data() + start, received_buffer.data() + sizeof(UDP_MessageHeader), message_size);
    return true;
}

uint64_t FileData::create_client_message(std::array<std::byte, MAX_LINE_SIZE> &send_buffer)
{
    auto part = std::begin(parts);
    std::advance(part, rand() % parts.size());
    // std::cout << "Send part " << *part << std::endl;

    auto first_point = *part * MAX_LINE_SIZE;
    auto last_point = std::min((uint64_t)(first_point + MAX_LINE_SIZE), (uint64_t)file_size);
    auto message_size = last_point - first_point;
    std::string bytes = std::string(&file_data[first_point],
                                    &file_data[last_point]);

    UDP_MessageHeader message_to_send{*part, static_cast<uint32_t>(parts_total),
                                      UDP_MessageHeader::Type::PUT, id};

    unsigned char message_data[bytes.size()];
    strcpy((char *)message_data, bytes.c_str());
    memcpy(send_buffer.data(), &message_to_send, sizeof(message_to_send));
    memcpy(send_buffer.data() + sizeof(message_to_send), file_data.data() + first_point, message_size);
    return message_size;
}

std::array<std::byte, BUFFER_ANSWER_SIZE> FileData::create_server_message(UDP_MessageHeader message)
{
    if (parts.size() == parts_total)
    {
        auto crc = crc32c(0, reinterpret_cast<unsigned char *>(file_data.data()), file_data.size());
        std::array<std::byte, BUFFER_ANSWER_SIZE> buffer_send;
        memcpy(&buffer_send, &message, sizeof(message));
        memcpy(&buffer_send[sizeof(message)], &crc, sizeof(crc));
        std::cout << "File received, crc = " << crc << std::endl;
        return buffer_send;
    }
    std::array<std::byte, BUFFER_ANSWER_SIZE> buffer_send;
    memcpy(&buffer_send, &message, sizeof(message));
    return buffer_send;
}
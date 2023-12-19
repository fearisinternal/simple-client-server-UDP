#include "filedata.h"
#include "logger.h"

bool FileData::open_file(const char *filename)
{
    std::ifstream file(filename, std::ios::binary);
    if (!file.is_open())
    {
        Logger::Error("File cannot be opened or does not exist");
        return false;
    }
    auto file_end = file.seekg(0, std::ios::end).tellg();
    file_size = file_end - file.seekg(0, std::ios::beg).tellg();
    file_data.resize(file_size);
    file.read(file_data.data(), file_data.size());

    Logger::Trace("Read file");
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

bool FileData::save_data_from_message(const UDP_MessageHeader &message,
                                      const std::array<std::byte, MAX_MESSAGE_SIZE> &received_buffer,
                                      const ssize_t &bytes_count)
{
    parts.insert(message.seq_number);
    parts_total = message.seq_total;
    int percent = parts.size() * 100 / parts_total;
    Logger::Info(("Received part: " + std::to_string(message.seq_number) + " (" + std::to_string(percent) + "%)").c_str());
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

    auto first_point = *part * MAX_LINE_SIZE;
    auto last_point = std::min((uint64_t)(first_point + MAX_LINE_SIZE), (uint64_t)file_size);
    auto message_size = last_point - first_point;
    std::string bytes = std::string(&file_data[first_point],
                                    &file_data[last_point]);

    UDP_MessageHeader message_to_send{*part, static_cast<uint32_t>(parts_total),
                                      UDP_MessageHeader::Type::PUT, id};

    memcpy(send_buffer.data(), &message_to_send, sizeof(message_to_send));
    memcpy(send_buffer.data() + sizeof(message_to_send), file_data.data() + first_point, message_size);

    Logger::Info(("Sent message: " + std::to_string(*part)).c_str());
    return message_size;
}

std::array<std::byte, BUFFER_ANSWER_SIZE> FileData::create_server_message(UDP_MessageHeader &message)
{
    message.seq_total = parts.size();
    message.type = UDP_MessageHeader::Type::ACK;
    if (parts.size() == parts_total)
    {
        auto crc = crc32c(0, reinterpret_cast<unsigned char *>(file_data.data()), file_data.size());
        std::array<std::byte, BUFFER_ANSWER_SIZE> buffer_send;
        memcpy(&buffer_send, &message, sizeof(message));
        memcpy(&buffer_send[sizeof(message)], &crc, sizeof(crc));
        Logger::Error(("The entire file is received, crc = " + std::to_string(crc)).c_str());
        return buffer_send;
    }
    std::array<std::byte, BUFFER_ANSWER_SIZE> buffer_send;
    memcpy(&buffer_send, &message, sizeof(message));
    return buffer_send;
}
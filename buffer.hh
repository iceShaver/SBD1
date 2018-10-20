//
// Created by kamil on 19.10.18.
//

#ifndef SBD_1_BUFFER_HH
#define SBD_1_BUFFER_HH


#include <array>
#include <filesystem>
#include <fstream>
#include <optional>
#include <string_view>
#include <algorithm>
#include <cstring>
#include <string>
#include "record.hh"

using Byte = char;

template<size_t _BufferSize>
class Buffer {
    static_assert(_BufferSize >= sizeof(Record::data_t), "Buffer size has to be >= record size");
    using Buffer_t = std::array<Byte, _BufferSize>;
public:
    enum class Mode {
        READ, WRITE
    };
    Buffer() = delete;
    Buffer(Buffer const &) = delete;
    Buffer(Buffer &&) = delete;
    Buffer &operator=(Buffer const &) = delete;
    Buffer &operator=(Buffer &&) = delete;
    ~Buffer();
    explicit Buffer(const char *path, Mode mode);
    std::optional<Record> ReadRecord();
    void WriteRecord(Record const &record);
    void ResetAndSetMode(Mode mode);
    size_t FreeBytes() const { return buffer_data.end() - buffer_iterator; }
    void Flush();
private:
    bool read_block();
    void write_block();

    Buffer_t buffer_data;
    typename Buffer_t::iterator buffer_iterator;
    typename Buffer_t::iterator eob_guard;  // End of Buffer, used only in read mode
    std::fstream file;
    Mode mode;
};

template<size_t _BufferSize>
void Buffer<_BufferSize>::ResetAndSetMode(Mode mode) {
    this->Flush();
    eob_guard = buffer_iterator = buffer_data.begin();
    file.seekg(0, std::ios::beg);
    file.seekp(0, std::ios::beg);
    file.clear();
    this->mode = mode;
}
template<size_t _BufferSize>
std::optional<Record> Buffer<_BufferSize>::ReadRecord() {
    if (mode != Mode::READ) {
        throw std::runtime_error("In order to read record you have to set buffer to reading mode.");
    }
    // TODO: Read record
    /*
     * Cases:
     * 1. buffer contains enough bytes (eob_guard - buffer_iter >= sizeof), return record
     * 2. buffer contains bytes partially (eob_guard - buffer_iter between (1, sizeof-1)), return record
     * 3. buffer contains no bytes (buffer_iter == eob)
     *  3.1 read from file if readable and return record
     *  3.2 return null if eof
     *
     */
    if (eob_guard - buffer_iterator >= sizeof(Record::data_t)) { // buffer contains enough bytes to be read;
        auto result = Record(*(Record::data_t *) (&(*buffer_iterator)));
        buffer_iterator += sizeof(Record::data_t);
        return result;
    }
    // buffer contains bytes partially, its needed to read new block
    if (eob_guard - buffer_iterator > 0 && eob_guard - buffer_iterator < sizeof(Record::data_t)) {
        if (file.eof()) { throw std::runtime_error("Buffer error: db file corrupted"); }
        auto bytes = std::vector<char>();
        bytes.reserve(sizeof(Record::data_t));
        auto bytesRead = eob_guard - buffer_iterator;
        std::copy(buffer_iterator, eob_guard, bytes.begin());
        if (!this->read_block()) { return std::nullopt; }
        std::copy(buffer_iterator, buffer_iterator + bytesRead, std::back_inserter(bytes));
        buffer_iterator += bytesRead;
        return Record(*((Record::data_t *) bytes.data()));
    }
    if (buffer_iterator == eob_guard && !file.eof()) {
        if (this->read_block()) { return ReadRecord(); }
        return std::nullopt;
    }
    return std::nullopt;
}

template<size_t _BufferSize>
void Buffer<_BufferSize>::WriteRecord(Record const &record) {
    if (mode != Mode::WRITE) {
        throw std::runtime_error("In order to write record you have to set buffer to writing mode");
    }
    // TODO: change to allow buffer size to be smaller than record size
    /*
     * Cases:
     * 1. Buffer contains enough space (eob - iter >= sizeof), writeBuffer
     * 2. Buffer contains space partially (eob - iter between (1, sizeof-1), writeBuffer
     * 3. Buffer contains no space, writeBuffer, write record
     */
    if (FreeBytes() >= sizeof(Record::data_t)) { // buffer contains enough space
        auto bytes = record.ToBytes();
        buffer_iterator = std::copy(bytes.begin(), bytes.end(), buffer_iterator);
        return;
    }
    if (FreeBytes() > 0 &&
        FreeBytes() < sizeof(Record::data_t)) { // buffer contains space partially
        auto bytes = record.ToBytes();
        int bytesCopied = buffer_data.end() - buffer_iterator;
        buffer_iterator = std::copy(bytes.begin(), bytes.begin() + bytesCopied, buffer_iterator); // write first part
        this->write_block();
        buffer_iterator = std::copy(bytes.begin() + bytesCopied, bytes.end(), buffer_iterator);   // write second part
        return;
    }
    if (buffer_iterator == buffer_data.end()) { // buffer contains no space
        this->write_block();
        this->WriteRecord(record);
        return;
    }
    throw std::runtime_error("Unable to write record, internal buffer error");
}
template<size_t _BufferSize>
bool Buffer<_BufferSize>::read_block() {
    if (file.eof()) { return false; }
    file.read(buffer_data.data(), buffer_data.size()); // care file with less bytes tha buffer size
    this->eob_guard = buffer_data.begin() + file.gcount();
    buffer_iterator = buffer_data.begin();
    return this->eob_guard > buffer_data.begin();
}
template<size_t _BufferSize>
void Buffer<_BufferSize>::write_block() {
    file.write(buffer_data.data(), buffer_iterator - buffer_data.begin()).flush();
    buffer_iterator = buffer_data.begin();
}
template<size_t _BufferSize>
Buffer<_BufferSize>::~Buffer() { this->Flush(); }
template<size_t _BufferSize>
Buffer<_BufferSize>::Buffer(const char *path, Buffer::Mode mode) : file(),
                                                                   mode(mode),
                                                                   buffer_data(Buffer_t()),
                                                                   buffer_iterator(buffer_data.begin()),
                                                                   eob_guard(buffer_data.begin()) {
    //file.exceptions(std::ifstream::failbit | std::ifstream::badbit);
    // TODO: change this in prod: remove prod, add app, tellg/p 0, maybe leave for write buffer
    file.open(path, std::ios::binary | std::ios::in | std::ios::out | std::ios::trunc);
}
template<size_t _BufferSize>
void Buffer<_BufferSize>::Flush() { if (buffer_iterator != buffer_data.begin()) { this->write_block(); }}

#endif //SBD_1_BUFFER_HH

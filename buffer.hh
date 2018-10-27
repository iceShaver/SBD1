//
// Created by kamil on 19.10.18.
//

#ifndef SBD_1_BUFFER_HH
#define SBD_1_BUFFER_HH


#include <array>
#include <fstream>
#include <optional>
#include <iostream>
#include <algorithm>
#include <vector>
#include <iomanip>
#include "record.hh"


template<size_t _BufferSize>
class Buffer {
    static_assert(_BufferSize >= sizeof(Record::data_t), "Buffer size has to be >= record size");
    using Buffer_t = std::array<uint8_t, _BufferSize>;

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

    auto ReadRecord() -> std::optional<Record>;
    void WriteRecord(Record const &record);
    void ResetAndSetMode(Mode mode);
    auto FreeBytes() const;
    auto RemainingBytes()const;
    void Flush();
    void PrintAllRecords();

private:
    std::fstream file;
    Mode mode;
    Buffer_t buffer_data;
    typename Buffer_t::iterator buffer_iterator;
    typename Buffer_t::iterator eob_guard;  // End of Buffer, used only in read mode

    bool read_block();
    void write_block();

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
auto Buffer<_BufferSize>::ReadRecord() -> std::optional<Record> {
    if (mode != Mode::READ) {
        throw std::runtime_error("In order to read record you have to set buffer to reading mode.");
    }

    if (RemainingBytes() >= sizeof(Record::data_t)) { // buffer contains enough bytes to be read;
        auto result = Record(*(Record::data_t *) (&(*buffer_iterator)));
        buffer_iterator += sizeof(Record::data_t);
        return result;
    }
    // buffer contains bytes partially, its needed to read new block
    if (RemainingBytes() > 0 && RemainingBytes() < sizeof(Record::data_t)) {
        if (file.eof()) { throw std::runtime_error("Buffer error: db file corrupted"); }
        auto bytes = std::vector<uint8_t>();
        bytes.reserve(sizeof(Record::data_t));
        auto bytesRead = RemainingBytes();
        std::copy(buffer_iterator, eob_guard, bytes.begin());
        if (!this->read_block()) { return std::nullopt; }
        std::copy(buffer_iterator, buffer_iterator + bytesRead, std::back_inserter(bytes));
        buffer_iterator += bytesRead;
        return Record(*reinterpret_cast<Record::data_t *> (bytes.data()));
    }
    if (buffer_iterator == eob_guard && !file.eof()) { // buffer contains no bytes, so we must read block from disk
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
    if (FreeBytes() >= sizeof(Record::data_t)) { // buffer contains enough space
        auto bytes = record.ToBytes();
        buffer_iterator = std::copy(bytes.begin(), bytes.end(), buffer_iterator);
        return;
    }
    if (FreeBytes() > 0 && FreeBytes() < sizeof(Record::data_t)) { // buffer contains space partially
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
    file.read((char *) buffer_data.data(), buffer_data.size()); // care file with less bytes tha buffer size
    this->eob_guard = buffer_data.begin() + file.gcount();
    buffer_iterator = buffer_data.begin();
    return this->eob_guard > buffer_data.begin();
}

template<size_t _BufferSize>
void Buffer<_BufferSize>::write_block() {
    file.write((const char *) buffer_data.data(), buffer_iterator - buffer_data.begin()).flush();
    buffer_iterator = buffer_data.begin();
}

template<size_t _BufferSize>
Buffer<_BufferSize>::~Buffer() { this->Flush(); }

template<size_t _BufferSize>
Buffer<_BufferSize>::Buffer(const char *path, Buffer::Mode mode) :
        file(path, std::ios::binary | std::ios::in | std::ios::out | std::ios::trunc),
        mode(mode),
        buffer_data(Buffer_t()),
        buffer_iterator(buffer_data.begin()),
        eob_guard(buffer_data.begin()) {}

template<size_t _BufferSize>
void Buffer<_BufferSize>::Flush() { if (buffer_iterator != buffer_data.begin()) { this->write_block(); }}

// TODO: check if ResetAndSetMode is safe in this context
template<size_t _BufferSize>
void Buffer<_BufferSize>::PrintAllRecords() {
    ResetAndSetMode(Mode::READ);
    auto col_width = 20;
    std::cout << std::setw(col_width) << "ID"
              << std::setw(col_width) << "Ocena 1"
              << std::setw(col_width) << "Ocena 2"
              << std::setw(col_width) << "Ocena 3"
              << std::setw(col_width) << "Średnia" << '\n';
    std::optional<Record> record;
    while ((record = this->ReadRecord())) { std::cout << *record << '\n'; }
}

template<size_t _BufferSize>
auto Buffer<_BufferSize>::FreeBytes() const {
    auto result = buffer_data.end() - buffer_iterator;
    return static_cast<long unsigned>(result > 0 ? result : 0);
}
template<size_t _BufferSize>
auto Buffer<_BufferSize>::RemainingBytes() const {
    auto result = eob_guard - buffer_iterator;
    return static_cast<long unsigned>(result > 0 ? result : 0);
}

#endif //SBD_1_BUFFER_HH

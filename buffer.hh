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

template<size_t _BufferSize> class Buffer;

template<size_t _BufferSize> std::ostream &operator<<(std::ostream &, const Buffer<_BufferSize> &);

template<size_t _BufferSize>
class Buffer {
    static_assert(_BufferSize >= sizeof(Record::data_t), "Buffer size has to be >= record size");
    using Buffer_t = std::array<uint8_t, _BufferSize>;

public:
    enum class Mode {
        READ, WRITE
    };
    enum class PrintMode {
        FULL, AVG_ONLY
    };

    Buffer() = delete;
    Buffer(Buffer const &) = delete;
    Buffer(Buffer &&) = delete;
    Buffer &operator=(Buffer const &) = delete;
    Buffer &operator=(Buffer &&) = delete;
    ~Buffer();
    explicit Buffer(const char *path, Mode mode);

    auto ReadRecord() -> std::optional<Record>;
    Buffer &WriteRecord(Record const &record);
    void ResetAndSetMode(Mode mode);
    auto FreeBytes() const;
    auto RemainingBytes() const;
    void Flush();
    void PrintAllRecords(PrintMode printMode = PrintMode::FULL);
    friend std::ostream &operator<<<_BufferSize>(std::ostream &os, const Buffer<_BufferSize> &buffer);

private:
    std::string path;
    std::fstream file;
    Mode mode;
    Buffer_t buffer_data;
    typename Buffer_t::iterator buffer_iterator;
    typename Buffer_t::iterator eob_guard;  // End of Buffer, used only in read mode
    uint64_t disk_reads_count;
    uint64_t disk_writes_count;
    bool read_block();
    void write_block();
};

template<size_t _BufferSize>
void Buffer<_BufferSize>::ResetAndSetMode(Mode mode) {
    this->Flush();
    eob_guard = buffer_iterator = buffer_data.begin();
    file.clear();
    file.close();
    switch (mode) {
        case Mode::READ:
            file.open(path, std::ios::binary | std::ios::in);
            break;
        case Mode::WRITE:
            file.open(path, std::ios::binary | std::ios::out | std::ios::trunc);
            break;
    }
/*    file.clear();
    file.seekg(0, std::ios::beg);
    file.seekp(0, std::ios::beg);*/
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
Buffer<_BufferSize> &Buffer<_BufferSize>::WriteRecord(Record const &record) {
    if (mode != Mode::WRITE) {
        throw std::runtime_error("In order to write record you have to set buffer to writing mode");
    }
    if (FreeBytes() >= sizeof(Record::data_t)) { // buffer contains enough space
        auto bytes = record.ToBytes();
        buffer_iterator = std::copy(bytes.begin(), bytes.end(), buffer_iterator);
        return *this;
    }
    if (FreeBytes() > 0 && FreeBytes() < sizeof(Record::data_t)) { // buffer contains space partially
        auto bytes = record.ToBytes();
        int bytesCopied = buffer_data.end() - buffer_iterator;
        buffer_iterator = std::copy(bytes.begin(), bytes.begin() + bytesCopied, buffer_iterator); // write first part
        this->write_block();
        buffer_iterator = std::copy(bytes.begin() + bytesCopied, bytes.end(), buffer_iterator);   // write second part
        return *this;
    }
    if (buffer_iterator == buffer_data.end()) { // buffer contains no space
        this->write_block();
        this->WriteRecord(record);
        return *this;
    }
    throw std::runtime_error("Unable to write record, internal buffer error");
}

template<size_t _BufferSize>
bool Buffer<_BufferSize>::read_block() {
    if (file.eof()) { return false; }
    ++this->disk_reads_count;
    file.read((char *) buffer_data.data(), buffer_data.size()); // care file with less bytes than buffer size
    this->eob_guard = buffer_data.begin() + file.gcount();
    buffer_iterator = buffer_data.begin();
    return this->eob_guard > buffer_data.begin();
}

template<size_t _BufferSize>
void Buffer<_BufferSize>::write_block() {
    ++this->disk_writes_count;
    file.write((const char *) buffer_data.data(), buffer_iterator - buffer_data.begin()).flush();
    buffer_iterator = buffer_data.begin();
}

template<size_t _BufferSize>
Buffer<_BufferSize>::~Buffer() { this->Flush(); }

template<size_t _BufferSize>
Buffer<_BufferSize>::Buffer(const char *path, Buffer::Mode mode) :
        path(path),
        // TODO: fix it
        mode(mode),
        buffer_data(Buffer_t()),
        buffer_iterator(buffer_data.begin()),
        eob_guard(buffer_data.begin()),
        disk_reads_count(0),
        disk_writes_count(0) {
    switch (mode) {
        case Mode::READ:
            file.open(path, std::ios::binary | std::ios::in);
            break;
        case Mode::WRITE:
            file.open(path, std::ios::binary | std::ios::out | std::ios::trunc);
            break;
    }

}

template<size_t _BufferSize>
void Buffer<_BufferSize>::Flush() {
    if (buffer_iterator != buffer_data.begin()) {
        this->write_block();
    }
}

// TODO: check if ResetAndSetMode is safe in this context
template<size_t _BufferSize>
void Buffer<_BufferSize>::PrintAllRecords(PrintMode printMode) {
    // make copy of all inner state variables
    this->Flush();
    auto f_g = this->file.tellg();
    auto f_p = this->file.tellp();
    auto f_state = this->file.rdstate();
    auto buffer_data = this->buffer_data;
    auto buffer_iterator_distance = this->buffer_iterator - this->buffer_data.begin();
    auto eob_guard_distance = this->eob_guard - this->buffer_data.begin();
    auto mode = this->mode;
    auto disk_reads_count = this->disk_reads_count;
    auto disk_writes_count = this->disk_writes_count;
    this->ResetAndSetMode(Mode::READ);
    auto col_width = 20;
    switch (printMode) {
        case PrintMode::FULL: {
            std::cout << std::setw(col_width) << "ID"
                      << std::setw(col_width) << "Ocena 1"
                      << std::setw(col_width) << "Ocena 2"
                      << std::setw(col_width) << "Ocena 3"
                      << std::setw(col_width) << "Średnia" << '\n';
            std::optional<Record> record;

            while ((record = this->ReadRecord())) { std::cout << *record << '\n'; }
            break;
        }
        case PrintMode::AVG_ONLY:
            std::optional<Record> record;
            while ((record = this->ReadRecord())) {
                std::cout << std::fixed << std::setprecision(2) << std::setw(5) << std::setfill('0') <<
                          record->GetAvg() << ' ';
            }
            std::cout << '\n';
            break;
    }
    // restore all inner state variables
    if (mode == Mode::WRITE) {
        file.close();
        file.clear();
        file.open(path, std::ios::binary | std::ios::out | std::ios::app);
    }
    this->file.seekg(f_g);
    this->file.seekp(f_p);
    this->file.setstate(f_state);
    this->buffer_data = buffer_data;
    this->buffer_iterator = this->buffer_data.begin() + buffer_iterator_distance;
    this->eob_guard = this->buffer_data.begin() + eob_guard_distance;
    this->mode = mode;
    this->disk_reads_count = disk_reads_count;
    this->disk_writes_count = disk_writes_count;
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
template<size_t _BufferSize>
std::ostream &operator<<(std::ostream &os, const Buffer<_BufferSize> &buffer) {
    return os << "Reads count: " << buffer.disk_reads_count << "\nWritesCount: " << buffer.disk_writes_count;
}

#endif //SBD_1_BUFFER_HH

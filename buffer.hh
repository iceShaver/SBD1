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
#include "config.hh"
#include <filesystem>

template<size_t _BufferSize> class Buffer;

template<size_t _BufferSize> std::ostream &operator<<(std::ostream &, const Buffer<_BufferSize> &);

namespace fs = std::filesystem;

template<size_t _BufferSize> class Buffer final {
    static_assert(_BufferSize >= sizeof(Record::data_t), "Buffer size has to be >= record size");
    using Buffer_t = std::array<uint8_t, _BufferSize>;

public:
    enum class Mode { READ, WRITE };
    enum class PrintMode { FULL, AVG_ONLY };

    Buffer() = delete;
    Buffer(Buffer const &) = delete;
    Buffer(Buffer &&)noexcept = default;
    Buffer &operator=(Buffer const &) = delete;
    Buffer &operator=(Buffer &&)noexcept = default ;
    ~Buffer();
    Buffer(const char *path, Mode mode, bool persistent_file = true);
    explicit Buffer(Mode mode);

    std::optional<Record> read_record();
    Buffer &write_record(Record const &record);
    void reset_and_set_mode(Mode mode);
    auto free_bytes() const;
    auto remaining_bytes() const;
    void flush();
    void print_all_records(PrintMode printMode = PrintMode::FULL);
    auto reads_count() const { return disk_reads_count; }
    auto writes_count() const { return disk_writes_count; }
    void reset_io_counters() { disk_reads_count = disk_writes_count = 0; }
    friend std::ostream &operator<<<_BufferSize>(std::ostream &os, const Buffer<_BufferSize> &buffer);

private:
    fs::path path;
    std::fstream file;
    Mode mode;
    Buffer_t buffer_data;
    typename Buffer_t::iterator buffer_iterator;
    typename Buffer_t::iterator eob_guard;  // End of Buffer, used only in read mode
    uint64_t disk_reads_count;
    uint64_t disk_writes_count;
    bool persistent_file;

    bool read_block();
    void write_block();
};

template<size_t _BufferSize> void Buffer<_BufferSize>::reset_and_set_mode(Mode mode) {
    this->flush();
    eob_guard = buffer_iterator = buffer_data.begin();
    file.clear();
    if (mode == Buffer::Mode::WRITE) { fs::resize_file(path, 0); }
    file.seekg(0);
    file.seekp(0);
    this->mode = mode;
}

template<size_t _BufferSize> std::optional<Record> Buffer<_BufferSize>::read_record()  {
    if (mode != Mode::READ) {
        throw std::runtime_error("In order to read record you have to set buffer to reading mode.");
    }

    if (remaining_bytes() >= sizeof(Record::data_t)) { // buffer contains enough bytes to be read;
        auto result = Record(*(Record::data_t *) (&(*buffer_iterator)));
        buffer_iterator += sizeof(Record::data_t);
        return result;
    }
    // buffer contains bytes partially, its needed to read new block
    if (remaining_bytes() > 0 && remaining_bytes() < sizeof(Record::data_t)) {
        if (file.eof()) { throw std::runtime_error("Buffer error: db file corrupted"); }
        auto bytes = std::vector<uint8_t>();
        bytes.reserve(sizeof(Record::data_t));
        auto bytes_read = remaining_bytes();
        std::copy(buffer_iterator, eob_guard, bytes.begin());
        if (!this->read_block()) { return std::nullopt; }
        std::copy(buffer_iterator, buffer_iterator + bytes_read, std::back_inserter(bytes));
        buffer_iterator += bytes_read;
        return Record(*reinterpret_cast<Record::data_t *> (bytes.data()));
    }
    if (buffer_iterator == eob_guard && !file.eof()) { // buffer contains no bytes, so we must read block from disk
        if (this->read_block()) { return read_record(); }
        return std::nullopt;
    }
    return std::nullopt;
}

template<size_t _BufferSize> Buffer<_BufferSize> &Buffer<_BufferSize>::write_record(Record const &record) {
    if (mode != Mode::WRITE) {
        throw std::runtime_error("In order to write record you have to set buffer to writing mode");
    }
    if (free_bytes() >= sizeof(Record::data_t)) { // buffer contains enough space
        auto bytes = record.to_bytes();
        buffer_iterator = std::copy(bytes.begin(), bytes.end(), buffer_iterator);
        return *this;
    }
    if (free_bytes() > 0 && free_bytes() < sizeof(Record::data_t)) { // buffer contains space partially
        auto bytes = record.to_bytes();
        int bytesCopied = buffer_data.end() - buffer_iterator;
        buffer_iterator = std::copy(bytes.begin(), bytes.begin() + bytesCopied, buffer_iterator); // write first part
        this->write_block();
        buffer_iterator = std::copy(bytes.begin() + bytesCopied, bytes.end(), buffer_iterator);   // write second part
        return *this;
    }
    if (buffer_iterator == buffer_data.end()) { // buffer contains no space
        this->write_block();
        this->write_record(record);
        return *this;
    }
    throw std::runtime_error("Unable to write record, internal buffer error");
}

template<size_t _BufferSize> bool Buffer<_BufferSize>::read_block() {
    if (file.eof()) { return false; }
    ++this->disk_reads_count;
    file.read((char *) buffer_data.data(), buffer_data.size()); // care file with less bytes than buffer size
    this->eob_guard = buffer_data.begin() + file.gcount();
    buffer_iterator = buffer_data.begin();
    return this->eob_guard > buffer_data.begin();
}

template<size_t _BufferSize> void Buffer<_BufferSize>::write_block() {
    ++this->disk_writes_count;
    file.write((const char *) buffer_data.data(), buffer_iterator - buffer_data.begin()).flush();
    buffer_iterator = buffer_data.begin();
}

template<size_t _BufferSize> Buffer<_BufferSize>::~Buffer() {
    this->flush();
    file.close();
    if (!persistent_file) fs::remove(path);
}

template<size_t _BufferSize> Buffer<_BufferSize>::Buffer(const char *path, Buffer::Mode mode, bool persistent_file) :
        path(path),
        file(path, std::ios::binary | std::ios::in | std::ios::out | std::ios::app),
        mode(mode),
        buffer_data(Buffer_t()),
        buffer_iterator(buffer_data.begin()),
        eob_guard(buffer_data.begin()),
        disk_reads_count(0),
        disk_writes_count(0),
        persistent_file(persistent_file) {
    file.exceptions(std::ios::badbit);
    if (mode == Buffer::Mode::WRITE) {
        fs::resize_file(this->path, 0);
        file.seekg(0);
        file.seekp(0);
    }
    if (Config::verbose) {
        std::cout << "Buffer file path" << ((persistent_file) ? " (persistent): " : ": ") << fs::absolute(this->path)
                  << std::endl;
    }
}
// TODO: check if constructing fs::path with mutable char* is safe
template<size_t _BufferSize>
Buffer<_BufferSize>::Buffer(Buffer::Mode mode) : Buffer(::tmpnam(nullptr), mode, false) {}

template<size_t _BufferSize> void Buffer<_BufferSize>::flush() {
    if (mode == Buffer::Mode::WRITE && buffer_iterator != buffer_data.begin()) {
        this->write_block();
    }
}

template<size_t _BufferSize> void Buffer<_BufferSize>::print_all_records(PrintMode printMode) {
    // make copy of all inner state variables

    this->flush();
    auto disk_reads_count = this->disk_reads_count;
    auto disk_writes_count = this->disk_writes_count;
    auto f_g = this->file.tellg();
    auto f_p = this->file.tellp();
    auto f_state = this->file.rdstate();
    auto buffer_data = this->buffer_data;
    auto buffer_iterator_distance = this->buffer_iterator - this->buffer_data.begin();
    auto eob_guard_distance = this->eob_guard - this->buffer_data.begin();
    auto mode = this->mode;

    this->reset_and_set_mode(Mode::READ);
    auto col_width = 20;
    switch (printMode) {
        case PrintMode::FULL: {
            std::cout << std::setw(col_width) << "ID"
                      << std::setw(col_width) << "Ocena 1"
                      << std::setw(col_width) << "Ocena 2"
                      << std::setw(col_width) << "Ocena 3"
                      << std::setw(col_width) << "Średnia" << '\n';
            std::optional<Record> record;

            while ((record = this->read_record())) { std::cout << *record << '\n'; }
            break;
        }
        case PrintMode::AVG_ONLY:
            std::optional<Record> record;
            while ((record = this->read_record())) {
                std::cout << std::fixed << std::setprecision(2) << std::setw(5) << std::setfill('0') <<
                                                                                                     record->get_avg() << ' ';
            }
            std::cout <<std::setfill(' ')<< '\n';
            break;
    }
    // restore all inner state variables
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

template<size_t _BufferSize> auto Buffer<_BufferSize>::free_bytes() const {
    auto result = buffer_data.end() - buffer_iterator;
    return static_cast<long unsigned>(result > 0 ? result : 0);
}

template<size_t _BufferSize> auto Buffer<_BufferSize>::remaining_bytes() const {
    auto result = eob_guard - buffer_iterator;
    return static_cast<long unsigned>(result > 0 ? result : 0);
}

template<size_t _BufferSize> std::ostream &operator<<(std::ostream &os, const Buffer<_BufferSize> &buffer) {
    return os << "Reads count: " << buffer.disk_reads_count << "\nWritesCount: " << buffer.disk_writes_count;
}


#endif //SBD_1_BUFFER_HH

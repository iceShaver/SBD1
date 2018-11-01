//
// Created by kamil on 01.11.18.
//

#ifndef SBD_1_SORTER_HH
#define SBD_1_SORTER_HH


#include <cstddef>
#include "buffer.hh"
#include "config.hh"

namespace Sorter {
    template<size_t _BufferSize> void natural_merge_sort_2_1(Buffer<_BufferSize> &buffer1) {
        auto buffers = std::array{
                Buffer<_BufferSize>("/home/kamil/Desktop/buf02", Buffer<_BufferSize>::Mode::WRITE),
                Buffer<_BufferSize>("/home/kamil/Desktop/buf03", Buffer<_BufferSize>::Mode::WRITE)
        };
        while (true) {
            buffer1.ResetAndSetMode(Buffer<_BufferSize>::Mode::READ);
            for (auto &&buf:buffers) { buf.ResetAndSetMode(Buffer<_BufferSize>::Mode::WRITE); }
            // distribute
            auto sorted = true;
            auto out_buf_switch = false;
            double prev_record_value{};
            std::optional<Record> rec{};
            while ((rec = buffer1.ReadRecord())) {
                if (rec->GetAvg() >= prev_record_value) {
                    buffers[out_buf_switch].WriteRecord(*rec);
                } else {
                    sorted = false;
                    buffers[out_buf_switch = !out_buf_switch].WriteRecord(*rec);
                }
                prev_record_value = rec->GetAvg();
            }
            if (sorted) { break; }
            if constexpr (Config::DEBUG) {
                buffer1.PrintAllRecords();
                for (auto &&buf:buffers) { buf.PrintAllRecords(); }
            }
            // merge
            buffer1.ResetAndSetMode(Buffer<_BufferSize>::Mode::WRITE);
            for (auto &&buf:buffers) { buf.ResetAndSetMode(Buffer<_BufferSize>::Mode::READ); }
            auto a = buffers[0].ReadRecord();
            auto b = buffers[1].ReadRecord();
            while (true) {
                if (a && b) {
                    if (a->GetAvg() < b->GetAvg()) {
                        buffer1.WriteRecord(*a);
                        a = buffers[0].ReadRecord();
                    } else {
                        buffer1.WriteRecord(*b);
                        b = buffers[1].ReadRecord();
                    }
                } else if (a) {
                    buffer1.WriteRecord(*a);
                    a = buffers[0].ReadRecord();
                } else if (b) {
                    buffer1.WriteRecord(*b);
                    b = buffers[1].ReadRecord();
                } else break;
            }
        }
        if constexpr (Config::DEBUG) { buffer1.PrintAllRecords(); }
    }
};


#endif //SBD_1_SORTER_HH

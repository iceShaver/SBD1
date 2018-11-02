//
// Created by kamil on 01.11.18.
//

#ifndef SBD_1_SORTER_HH
#define SBD_1_SORTER_HH


#include <cstddef>
#include "buffer.hh"
#include "config.hh"

namespace Sorter {
    template<size_t _BufferSize> auto natural_merge_sort_2_1(Buffer<_BufferSize> &buffer1) {
        buffer1.reset_io_counters();
        auto buffers = std::array{ // TODO: change file paths to tmp files
                Buffer<_BufferSize>(Buffer<_BufferSize>::Mode::WRITE),
                Buffer<_BufferSize>(Buffer<_BufferSize>::Mode::WRITE)
        };
        if (Config::debug)buffer1.PrintAllRecords(Buffer<_BufferSize>::PrintMode::AVG_ONLY);
        auto iter_counter = 0u;
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
            if (Config::debug) {
                buffer1.PrintAllRecords(Buffer<_BufferSize>::PrintMode::AVG_ONLY);
                for (auto &&buf:buffers) { buf.PrintAllRecords(Buffer<_BufferSize>::PrintMode::AVG_ONLY); }
            }
            // merge
            buffer1.ResetAndSetMode(Buffer<_BufferSize>::Mode::WRITE);
            for (auto &&buf:buffers) { buf.ResetAndSetMode(Buffer<_BufferSize>::Mode::READ); }
            auto a = buffers[0].ReadRecord();
            auto b = buffers[1].ReadRecord();
            auto prev_a = std::optional<Record>{};
            auto prev_b = std::optional<Record>{};
            auto eor_a = false; // End Of Run
            auto eor_b = false;
            while (true) {
                prev_a = a;
                prev_b = b;
                if (eor_a && eor_b) {
                    eor_a = eor_b = false;
                } else if (a && !eor_a && b && !eor_b) {
                    if (a->GetAvg() < b->GetAvg()) {
                        buffer1.WriteRecord(*a);
                        a = buffers[0].ReadRecord();
                        if (a->GetAvg() < prev_a->GetAvg()) { eor_a = true; }
                    } else {
                        buffer1.WriteRecord(*b);
                        b = buffers[1].ReadRecord();
                        if (b->GetAvg() < prev_b->GetAvg()) { eor_b = true; }
                    }
                } else if (a && !eor_a) {
                    buffer1.WriteRecord(*a);
                    a = buffers[0].ReadRecord();
                    if (a->GetAvg() < prev_a->GetAvg()) { eor_a = true; }
                } else if (b && !eor_b) {
                    buffer1.WriteRecord(*b);
                    b = buffers[1].ReadRecord();
                    if (b->GetAvg() < prev_b->GetAvg()) { eor_b = true; }
                } else if (a) {
                    eor_a = false;
                } else if (b) {
                    eor_b = false;
                } else break;
            }
            if (Config::debug) { buffer1.PrintAllRecords(Buffer<_BufferSize>::PrintMode::AVG_ONLY); }
            ++iter_counter;
        }
        return std::tuple(iter_counter,
                          buffer1.reads_count() + buffers[0].reads_count() + buffers[1].reads_count(),
                          buffer1.writes_count() + buffers[0].writes_count() + buffers[1].writes_count());
    }

    template<size_t _BufferSize> void polyphase_sort_3(Buffer<_BufferSize> &buffer1) {

    }
}


#endif //SBD_1_SORTER_HH

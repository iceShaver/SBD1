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
        buffer1.reset_and_set_mode(Buffer<_BufferSize>::Mode::READ);
        buffer1.reset_io_counters();
        auto buffers = std::array{
                Buffer<_BufferSize>(Buffer<_BufferSize>::Mode::WRITE),
                Buffer<_BufferSize>(Buffer<_BufferSize>::Mode::WRITE)
        };
        auto iter_counter = 0u;
        while (true) {
            buffer1.reset_and_set_mode(Buffer<_BufferSize>::Mode::READ);
            for (auto &&buf:buffers) { buf.reset_and_set_mode(Buffer<_BufferSize>::Mode::WRITE); }
            // distribute
            auto out_buf_switch = false;
            double prev_rec_avg{};
            std::optional<Record> rec{};
            while ((rec = buffer1.read_record())) {
                if (rec->get_avg() >= prev_rec_avg) {
                    buffers[out_buf_switch].write_record(*rec);
                } else {
                    buffers[out_buf_switch = !out_buf_switch].write_record(*rec);
                }
                prev_rec_avg = rec->get_avg();
            }
            if (Config::debug) {
                buffer1.print_all_records(Buffer<_BufferSize>::PrintMode::AVG_ONLY);
                for (auto &&buf:buffers) { buf.print_all_records(Buffer<_BufferSize>::PrintMode::AVG_ONLY); }
            }
            // merge
            buffer1.reset_and_set_mode(Buffer<_BufferSize>::Mode::WRITE);
            for (auto &&buf:buffers) { buf.reset_and_set_mode(Buffer<_BufferSize>::Mode::READ); }
            auto a = buffers[0].read_record();
            auto b = buffers[1].read_record();
            auto prev_a = std::optional<Record>{};
            auto prev_b = std::optional<Record>{};
            auto eor_a = false; // End Of Run
            auto eor_b = false;
            auto sorted = true;
            while (true) {
                prev_a = a;
                prev_b = b;
                if (eor_a && eor_b) {
                    sorted = eor_a = eor_b = false;
                } else if (a && !eor_a && b && !eor_b) {
                    if (a->get_avg() < b->get_avg()) {
                        buffer1.write_record(*a);
                        a = buffers[0].read_record();
                        if (a->get_avg() < prev_a->get_avg()) { eor_a = true; }
                    } else {
                        buffer1.write_record(*b);
                        b = buffers[1].read_record();
                        if (b->get_avg() < prev_b->get_avg()) { eor_b = true; }
                    }
                } else if (a && !eor_a) {
                    buffer1.write_record(*a);
                    a = buffers[0].read_record();
                    if (a->get_avg() < prev_a->get_avg()) { eor_a = true; }
                } else if (b && !eor_b) {
                    buffer1.write_record(*b);
                    b = buffers[1].read_record();
                    if (b->get_avg() < prev_b->get_avg()) { eor_b = true; }
                } else if (a) {
                    eor_a = false;
                } else if (b) {
                    eor_b = false;
                } else break;
            }
            ++iter_counter;
            if (sorted) break;
        }
        if (Config::debug) { buffer1.print_all_records(Buffer<_BufferSize>::PrintMode::AVG_ONLY); }
        return std::tuple(iter_counter,
                          buffer1.reads_count() + buffers[0].reads_count() + buffers[1].reads_count(),
                          buffer1.writes_count() + buffers[0].writes_count() + buffers[1].writes_count());
    }
}


#endif //SBD_1_SORTER_HH

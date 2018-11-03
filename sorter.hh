//
// Created by kamil on 01.11.18.
//

#ifndef SBD_1_SORTER_HH
#define SBD_1_SORTER_HH


#include <cstddef>
#include "buffer.hh"
#include "config.hh"

namespace Sorter {
    template<size_t _BufferSize> auto natural_merge_sort_2_1(Buffer<_BufferSize> &buf1) {
        using Buffer_t = Buffer<_BufferSize>;
        buf1.reset_and_set_mode(Buffer_t::Mode::READ);
        buf1.reset_io_counters();
        auto buffs = std::array{
                Buffer_t(Buffer_t::Mode::WRITE),
                Buffer_t(Buffer_t::Mode::WRITE)
        };
        auto iter_counter = 0u;
        while (true) {
            buf1.reset_and_set_mode(Buffer_t::Mode::READ);
            for (auto &&buf:buffs) { buf.reset_and_set_mode(Buffer_t::Mode::WRITE); }
            // distribute
            auto out_buf_switch = false;
            double prev_rec_avg{};
            std::optional<Record> rec{};
            while ((rec = buf1.read_record())) {
                if (rec->get_avg() >= prev_rec_avg) {
                    buffs[out_buf_switch].write_record(*rec);
                } else {
                    buffs[out_buf_switch = !out_buf_switch].write_record(*rec);
                }
                prev_rec_avg = rec->get_avg();
            }
            debug([&] {
                buf1.print_all_records(Buffer_t::PrintMode::AVG_ONLY);
                for (auto &&buf:buffs) { buf.print_all_records(Buffer_t::PrintMode::AVG_ONLY); }
            });

            // merge
            buf1.reset_and_set_mode(Buffer_t::Mode::WRITE);
            for (auto &&buf:buffs) { buf.reset_and_set_mode(Buffer_t::Mode::READ); }
            auto a = buffs[0].read_record();
            auto b = buffs[1].read_record();
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
                        buf1.write_record(*a);
                        a = buffs[0].read_record();
                        if (a->get_avg() < prev_a->get_avg()) { eor_a = true; }
                    } else {
                        buf1.write_record(*b);
                        b = buffs[1].read_record();
                        if (b->get_avg() < prev_b->get_avg()) { eor_b = true; }
                    }
                } else if (a && !eor_a) {
                    buf1.write_record(*a);
                    a = buffs[0].read_record();
                    if (a->get_avg() < prev_a->get_avg()) { eor_a = true; }
                } else if (b && !eor_b) {
                    buf1.write_record(*b);
                    b = buffs[1].read_record();
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
        debug([&] { buf1.print_all_records(Buffer_t::PrintMode::AVG_ONLY); });
        return std::tuple{
                iter_counter,
                buf1.get_disk_reads_count() + buffs[0].get_disk_reads_count() + buffs[1].get_disk_reads_count(),
                buf1.get_disk_writes_count() + buffs[0].get_disk_writes_count() + buffs[1].get_disk_writes_count(),
                buf1.get_records_reads_count() + buffs[0].get_records_reads_count() +
                buffs[1].get_records_reads_count(),
                buf1.get_records_writes_count() + buffs[0].get_records_writes_count() +
                buffs[1].get_records_writes_count()
        };
    }
}


#endif //SBD_1_SORTER_HH

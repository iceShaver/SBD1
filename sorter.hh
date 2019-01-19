//
// Created by kamil on 01.11.18.
//

#ifndef SBD_1_SORTER_HH
#define SBD_1_SORTER_HH


#include <cstddef>
#include "buffer.hh"
#include "config.hh"

namespace Sorter {
    template<size_t _BufferSize>
    auto natural_merge_sort_2_1(Buffer<_BufferSize> &buf1) -> std::tuple<uint, uint64_t, uint64_t, uint64_t, uint64_t> {
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
            for (auto &&buf:buffs) buf.reset_and_set_mode(Buffer_t::Mode::WRITE);

            // distribute
            bool out_buf_switch = false;
            double prev_rec_avg = 0.0;
            auto rec = std::optional<Record>{};
            while ((rec = buf1.read_record())) {
                if (rec->get_avg() >= prev_rec_avg) buffs[out_buf_switch].write_record(*rec);
                else buffs[out_buf_switch = !out_buf_switch].write_record(*rec);
                prev_rec_avg = rec->get_avg();
            }

            debug([&] {
                std::cout << "Buffer 1: ";
                buf1.print_all_records(Buffer_t::PrintMode::AVG_ONLY);
                std::cout << "Buffer 2: ";
                buffs[0].print_all_records(Buffer_t::PrintMode::AVG_ONLY);
                std::cout << "Buffer 3: ";
                buffs[1].print_all_records(Buffer_t::PrintMode::AVG_ONLY);
            });

            // merge
            buf1.reset_and_set_mode(Buffer_t::Mode::WRITE);
            for (auto &&buf:buffs) { buf.reset_and_set_mode(Buffer_t::Mode::READ); }
            auto last_written = std::optional<Record>{};
            bool sorted = true;
            auto a = buffs[0].read_record();
            auto b = buffs[1].read_record();
            while (true) {
                if (!(a || b)) { break; }
                if (a && (!b || a <= b)) { // if a < b -> write a and get next from buf 0
                    if (last_written && last_written > a) sorted = false;
                    buf1.write_record(*(last_written = std::exchange(a, buffs[0].read_record())));
                    continue;
                }
                if (b && (!a || a > b)) { // if a > b write b and get next from buf 1;
                    if (last_written && last_written > b) sorted = false;
                    buf1.write_record(*(last_written = std::exchange(b, buffs[1].read_record())));
                }
            }


            ++iter_counter;
            if (sorted) break;
        }

        debug([&] {
            std::cout << "Buffer 1: ";
            buf1.print_all_records(Buffer_t::PrintMode::AVG_ONLY);
        });

        return {iter_counter,
                buf1.get_disk_r_count() + buffs[0].get_disk_r_count() + buffs[1].get_disk_r_count(),
                buf1.get_disk_w_count() + buffs[0].get_disk_w_count() + buffs[1].get_disk_w_count(),
                buf1.get_rec_r_count() + buffs[0].get_rec_r_count() + buffs[1].get_rec_r_count(),
                buf1.get_rec_w_count() + buffs[0].get_rec_w_count() + buffs[1].get_rec_w_count()
        };
    }
}


#endif //SBD_1_SORTER_HH

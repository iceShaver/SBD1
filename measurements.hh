//
// Created by kamil on 03.11.18.
//

#ifndef SBD_1_MEASUREMENTS_HH
#define SBD_1_MEASUREMENTS_HH


#include <cstddef>
#include "buffer.hh"
#include "records_generator.hh"
#include "sorter.hh"

namespace Measurements {
    namespace {
        constexpr auto const BUFFER_SIZE = 4096;
        using Buffer_t = Buffer<BUFFER_SIZE>;
    }

    void io() {
        using std::cout, std::endl, std::setw;
        // set_size, iters, disk_reads, disk_writes, rec_reads, rec_writes
        auto results = std::vector<std::tuple<uint64_t, uint64_t, uint64_t, uint64_t, uint64_t>>();
        constexpr auto const OUT_FILENAME = "measurements_results.csv";
        constexpr auto const DELIM = ',';
        constexpr auto const COL_WIDTH = 10;
        auto result_file = std::fstream(OUT_FILENAME, std::ios::out | std::ios::trunc);

        auto data_sets = std::array{
                100, 1'000, 10'000, 100'000, 1'000'000//, 10'000'000//, 100'000'000, 1'000'000'000
        };
        result_file << "N,iterations,disk_r,disk_w,rec_r,rec_w\n";
        verbose([] {
            std::cout << setw(COL_WIDTH) << "N" << setw(COL_WIDTH) << "iters" << setw(COL_WIDTH) << "disk_r"
                      << setw(COL_WIDTH) << "disk_w" << setw(COL_WIDTH) << "rec_r" << setw(COL_WIDTH) << "rec_w"
                      << endl;
        });
        for (auto &&data_set:data_sets) {
            auto buf = Buffer_t(Buffer_t::Mode::WRITE);
            RecordsGenerator::random(data_set, buf);
            auto[iters, disk_r, disk_w, rec_r, rec_w] = Sorter::natural_merge_sort_2_1(buf);
            result_file << data_set << DELIM << iters << DELIM << disk_r << DELIM << disk_w << DELIM << rec_r << DELIM
                        << rec_w << '\n';
            verbose([&] {
                std::cout << std::setw(COL_WIDTH) << data_set << std::setw(COL_WIDTH) << iters << std::setw(COL_WIDTH)
                          << disk_r << std::setw(COL_WIDTH) << disk_w << std::setw(COL_WIDTH) << rec_r
                          << std::setw(COL_WIDTH) << rec_w << '\n';
            });
        }
        std::cout << "Results saved in: " + fs::absolute(OUT_FILENAME).string() << std::endl;
    }
};


#endif //SBD_1_MEASUREMENTS_HH

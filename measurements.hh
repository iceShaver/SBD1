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
        // set_size, iters, disk_reads, disk_writes, rec_reads, rec_writes
        auto results = std::vector<std::tuple<uint64_t, uint64_t, uint64_t, uint64_t, uint64_t>>();
        constexpr auto const out_filename = "measurements_results.csv";
        auto result_file = std::fstream(out_filename, std::ios::trunc);
        constexpr auto const DELIM = ',';
        auto data_sets = std::array{
                100, 1'000, 10'000, 100'000, 1'000'000, 10'000'000, 100'000'000, 1'000'000'000
        };
        for (auto &&data_set:data_sets) {
            verbose([&]{ std::cout << data_set << std::endl;}); // TODO: random generate
            auto buf = Buffer_t(Buffer_t::Mode::WRITE);
            auto[iters, disk_r, disk_w, rec_r, rec_w] = Sorter::natural_merge_sort_2_1(buf);
            result_file << data_set << DELIM << iters << DELIM
                        << disk_r << DELIM << disk_w << DELIM
                        << rec_r << DELIM << rec_w;
        }
        verbose([&]{std::cout << "Results saved in: " + fs::absolute(out_filename).string() << std::endl;});
    }
};


#endif //SBD_1_MEASUREMENTS_HH

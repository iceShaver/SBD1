//
// Created by kamil on 03.11.18.
//

#ifndef SBD_1_MEASUREMENTS_HH
#define SBD_1_MEASUREMENTS_HH


#include <cstddef>
#include "buffer.hh"
#include "records_generator.hh"
#include "sorter.hh"
#include <future>

namespace Measurements {
    namespace {
        using Buffer_t = Buffer<Config::BUFFER_SIZE>;

        struct separator final : std::numpunct<char> {
            auto do_grouping() const -> std::string override { return "\03"; }
            auto do_thousands_sep() const -> char override { return ' '; }
        };
    }

    auto io() -> void {
        using std::cout, std::endl, std::setw;
        // set_size, iters, disk_reads, disk_writes, rec_reads, rec_writes
        auto results = std::vector<std::tuple<uint64_t, uint64_t, uint64_t, uint64_t, uint64_t>>();
        constexpr auto const DELIM = ',';
        constexpr auto const COL_WIDTH = 15;
        auto result_file = std::fstream(Config::MEASUREMENTS_OUT_FILENAME, std::ios::out | std::ios::trunc);
        auto data_sets = std::array{
                100, 500,
                1'000, 5'000,
                10'000, 50'000,
                100'000, 500'000,
                1'000'000, 5'000'000,
                10'000'000
        };
        result_file << "N,iterations,disk_r,disk_w,rec_r,rec_w,total_disk_io,theory_disk_io\n";
        auto theoretical_disk_io = [](auto N) {
            return static_cast<uint64_t>(
                    4 * N * std::ceil(std::log2(N / 2.0)) /
                    (Config::BUFFER_SIZE / static_cast<double>(sizeof(Record::data_t))));
        };
        std::cout.imbue(std::locale(std::cout.getloc(), new separator));
        std::cout << setw(COL_WIDTH) << "N" << setw(COL_WIDTH) << "iters" << setw(COL_WIDTH) << "disk_r"
                  << setw(COL_WIDTH) << "disk_w" << setw(COL_WIDTH) << "rec_r" << setw(COL_WIDTH) << "rec_w"
                  << setw(COL_WIDTH) << "disk_io" << setw(COL_WIDTH) << "th_disk_io" << endl;
        for (auto &&N:data_sets) {
            auto buf = Buffer_t(Buffer_t::Mode::WRITE);
            RecordsGenerator::random(N, buf);
            auto[iters, disk_r, disk_w, rec_r, rec_w] = Sorter::natural_merge_sort_2_1(buf);
            result_file << N << DELIM << iters << DELIM << disk_r << DELIM << disk_w << DELIM << rec_r << DELIM
                        << rec_w << DELIM << disk_r + disk_w << DELIM << theoretical_disk_io(N) << '\n';
            std::cout << std::setw(COL_WIDTH) << N << std::setw(COL_WIDTH) << iters << std::setw(COL_WIDTH)
                      << disk_r << std::setw(COL_WIDTH) << disk_w << std::setw(COL_WIDTH) << rec_r
                      << std::setw(COL_WIDTH) << rec_w << std::setw(COL_WIDTH) << disk_r + disk_w
                      << std::setw(COL_WIDTH) << theoretical_disk_io(N) << '\n';

        }
        std::cout << "Results saved in: " + fs::absolute(Config::MEASUREMENTS_OUT_FILENAME).string() << std::endl;
    }
}


#endif //SBD_1_MEASUREMENTS_HH

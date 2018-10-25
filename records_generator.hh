//
// Created by kamil on 20.10.18.
//

#ifndef SBD_1_RECORDS_GENERATOR_HH
#define SBD_1_RECORDS_GENERATOR_HH


#include <random>
#include "buffer.hh"

namespace RecordsGenerator {
    // TODO: move it to the Record
    template<size_t _BufferSize>
    void Random(unsigned long long n, Buffer<_BufferSize> &buffer) {
        std::random_device rd;
        std::mt19937_64 gen(rd());
        std::uniform_int_distribution<uint32_t> uid(2, 5);
        for (auto i = 0u; i < n; ++i) {
            buffer.WriteRecord({i, (uint8_t)uid(gen), (uint8_t)uid(gen), (uint8_t)uid(gen)});
        }
    }

}


#endif //SBD_1_RECORDS_GENERATOR_HH

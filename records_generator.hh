//
// Created by kamil on 20.10.18.
//

#ifndef SBD_1_RECORDS_GENERATOR_HH
#define SBD_1_RECORDS_GENERATOR_HH


#include <random>
#include "buffer.hh"

namespace RecordsGenerator {
    template<size_t _BufferSize>
    void Random(unsigned long long n, Buffer<_BufferSize> &buffer) {
        std::random_device rd;
        std::mt19937_64 gen(rd());
        std::uniform_int_distribution<> uid(2, 5);
        for (int i = 0; i < n; ++i) {
            buffer.WriteRecord({i, uid(gen), uid(gen), uid(gen)});
        }
    }

}


#endif //SBD_1_RECORDS_GENERATOR_HH

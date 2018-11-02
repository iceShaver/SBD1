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
        for (auto i = 0u; i < n; ++i) {
            buffer.WriteRecord(Record::Random());
        }
    }
    template<size_t _BufferSize> void ReadFromKeyboard(Buffer<_BufferSize> &buffer) {
        std::cout << "\nWpisz Ocena1 Ocena2 Ocena3, ctrl+d aby zakończyć\n";
        unsigned a, b, c;
        while (true) {
            std::cin >> a >> b >> c;
            if (std::cin.eof())break;
            auto record = Record{static_cast<uint8_t>(a), static_cast<uint8_t>(b), static_cast<uint8_t>(c)};
            buffer.WriteRecord(record);
            std::cout <<"Zapisano: " <<  record << std::endl;
        }
        std::cout << "Wpisane rekordy:\n";
        buffer.PrintAllRecords(Buffer<_BufferSize>::PrintMode::FULL);
    }
}


#endif //SBD_1_RECORDS_GENERATOR_HH

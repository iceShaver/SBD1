//
// Created by kamil on 20.10.18.
//

#ifndef SBD_1_RECORDS_GENERATOR_HH
#define SBD_1_RECORDS_GENERATOR_HH


#include <random>
#include "buffer.hh"
#include <string>
#include <iostream>
#include <filesystem>
#include <fstream>
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string.hpp>

namespace RecordsGenerator {
    using std::cout, std::cerr, std::cin, std::endl, std::string, std::fstream,
    std::ios;
    namespace fs = std::filesystem;
    enum class Source { RANDOM, USER_INPUT, CSV_FILE };

    template<size_t _BufferSize> void random(unsigned long long n, Buffer<_BufferSize> &buffer) {
        for (auto i = 0u; i < n; ++i) {
            buffer.write_record(Record::random());
        }
    }

    template<size_t _BufferSize> void from_keyboard(Buffer<_BufferSize> &buffer) {
        unsigned a, b, c;
        while (true) {
            cin >> a >> b >> c;
            if (cin.eof())break;
            try {
                auto record = Record{static_cast<uint8_t>(a), static_cast<uint8_t>(b), static_cast<uint8_t>(c)};
                buffer.write_record(record);
                cout << "Saved record: " << record << endl;
            } catch (std::invalid_argument &e) {
                cerr << "Error while creating record: " << e.what() << endl;
            }


        }
    }

    template<size_t _BufferSize> void from_file(Buffer<_BufferSize> &buffer, fs::path const &file_path) {
        /*auto file = fstream(file_path, ios::in);
        auto to_record = [](auto str) {
            auto result = std::vector<string>(3);
            boost::split(result, str, boost::is_any_of(","));
            return Record{static_cast<uint8_t>(std::stoi(result[0])),
                          static_cast<uint8_t>(std::stoi(result[1])),
                          static_cast<uint8_t>(std::stoi(result[2]))};
        };
        string line;
        while (!std::getline(file, line).eof()) {
            buffer.write_record(to_record(line));
        }*/
        fs::copy(file_path, buffer.get_buffer_file_path());
        buffer.reset_and_set_mode(Buffer<_BufferSize>::Mode::READ);
    }

    void generate_test_files() {
        constexpr auto const FILES_N = 10;
        auto tests_dir = fs::path{"test_files"};
        if (!fs::exists(tests_dir))fs::create_directory(tests_dir);
        for (int j = 0; j < FILES_N; ++j) {
            auto buf = Buffer<Config::BUFFER_SIZE>{tests_dir / (std::to_string(j) + ".bin"),
                                                   Buffer<Config::BUFFER_SIZE>::Mode::WRITE};
            RecordsGenerator::random(static_cast<uint64_t >(std::pow(j + 1, 2)), buf);
            buf.flush();
        }
    }
}


#endif //SBD_1_RECORDS_GENERATOR_HH

//
// Created by kamil on 01.11.18.
//

#ifndef SBD_1_CONFIG_HH
#define SBD_1_CONFIG_HH
namespace Config {
    constexpr auto const BUFFER_SIZE = 4096;
    constexpr auto const MEASUREMENTS_OUT_FILENAME = "measurements_results.csv";
    constexpr auto const TEST_FILES_N = 10;
    auto debug = false;
    auto verbose = false;
}
#endif //SBD_1_CONFIG_HH

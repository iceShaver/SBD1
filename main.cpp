#include <iostream>
#include <bitset>
#include "record.hh"
#include "buffer.hh"
#include "records_generator.hh"
#include "sorter.hh"
#include <boost/program_options.hpp>
#include <filesystem>

/*
 * WARNING: Program is currently valid only for LE machines
 */
using Buffer_t = Buffer<4096 * 64>;
namespace po = boost::program_options;
namespace fs = std::filesystem;

int main(int argc, char **argv) {
    auto file_path_string = std::string{};
    auto file_path = fs::path{};
    auto output_file_path_string = std::string{};
    auto n_random_records = size_t{};
    auto file_selected = false;
    auto random_selected = false;
    auto user_selected = false;
    auto output_file_selected = false;
    try {
        auto desc = po::options_description{"Allowed options"};
        desc.add_options()
                ("help,h", "display help")
                ("file,f", po::value(&file_path_string), "choose file as data source")
                ("random,r", po::value(&n_random_records), "generate N random records")
                ("user,u", "user input for records generation")
                ("verbose,v", "prints data after every stage of sorting")
                ("output,o", po::value(&output_file_path_string), "set output file");
        auto vm = po::variables_map{};
        po::store(po::parse_command_line(argc, argv, desc), vm);
        po::notify(vm);
        if (vm.count("help")) {
            std::cout << desc << '\n';
            return 0;
        }
        if (vm.count("file")) {
            file_path = fs::path{file_path_string};
            if (!fs::is_regular_file(file_path)) {
                std::cerr << "Specified file does not exists or is not a file." << std::endl;
                return -1;
            }
            file_selected = true;
        }
        if (vm.count("output")) {
            output_file_selected = true;
        }
        if (vm.count("random")) { random_selected = true; }
        if (vm.count("user")) { user_selected = true; }
        if (vm.count("verbose")) { Config::verbose = true; }
    } catch (po::error &e) {
        std::cerr << "Error parsing program arguments\n" << e.what() << std::endl;
        return -1;
    }
    auto buf = std::unique_ptr<Buffer_t>{};

    if (output_file_selected)
        buf = std::make_unique<Buffer_t>(output_file_path_string.c_str(), Buffer_t::Mode::WRITE);
    else
        buf = std::make_unique<Buffer_t>(Buffer_t::Mode::WRITE);

    if (file_selected) {
        RecordsGenerator::from_csv_file(*buf, file_path);
    }
    if (random_selected) {
        RecordsGenerator::random(n_random_records, *buf);
    }
    if (user_selected) {
        RecordsGenerator::from_keyboard(*buf);
    }

    Sorter::natural_merge_sort_2_1(*buf);
    return 0;
}
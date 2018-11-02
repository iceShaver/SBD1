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

int main(int argc, char **argv) {
    using std::cout, std::cerr, std::endl, std::string;
    namespace po = boost::program_options;
    namespace fs = std::filesystem;
    string file_path;
    size_t n_random_records;
    auto file_selected = false;
    auto random_selected = false;
    auto user_selected = false;
    try {
        po::options_description desc("Allowed options");
        desc.add_options()
                ("help,h", "display help")
                ("file,f", po::value(&file_path), "choose file as data source")
                ("random,r", po::value(&n_random_records), "generate N random records")
                ("user,u", "user input for records generation")
                ("verbose,v", "prints data after every stage of sorting");
        po::variables_map vm;
        po::store(po::parse_command_line(argc, argv, desc), vm);
        po::notify(vm);
        if (vm.count("help")) {
            cout << desc << '\n';
            return 0;
        }
        if (vm.count("file")) {
            auto file = fs::path(file_path);
            if (!fs::is_regular_file(file)) {
                cerr << "Specified file does not exists or is not a file." << endl;
                return -1;
            }
            file_selected = true;
        }
        if (vm.count("random")) { random_selected = true; }
        if (vm.count("user")) { user_selected = true; }
        if (vm.count("verbose")) { Config::verbose = true; }
    } catch (po::error &e) {
        cerr << "Error parsing program arguments\n";
        cerr << e.what() << endl;
        return -1;
    }
    Buffer_t buf("./buf01", Buffer_t::Mode::WRITE);
    if (file_selected) {
        RecordsGenerator::from_csv_file(buf, file_path);
    }
    if (random_selected) {
        RecordsGenerator::random(n_random_records, buf);
    }
    if (user_selected) {
        RecordsGenerator::from_keyboard(buf);
    }

    Sorter::natural_merge_sort_2_1(buf);
    return 0;
}
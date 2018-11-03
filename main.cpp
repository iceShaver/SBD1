#include <iostream>
#include <bitset>
#include "record.hh"
#include "buffer.hh"
#include "records_generator.hh"
#include "sorter.hh"
#include "terminal.hh"
#include <boost/program_options.hpp>
#include <filesystem>
#include "measurements.hh"

using Buffer_t = Buffer<16>;
using std::cout, std::cerr,
std::endl;
namespace po = boost::program_options;
namespace fs = std::filesystem;


enum class Action { NOT_SELECTED, SORT_CSV_FILE, SORT_RANDOM_DATA, SORT_USER_INPUT_DATA, MEASURE };

/*
 * WARNING: Program is currently valid only for LE machines
 */
int main(int argc, char **argv) {

    // Parse program arguments
    auto action = Action::NOT_SELECTED;
    auto file_path_string = std::string{};
    auto file_path = fs::path{};
    auto output_file_path_string = std::string{};
    auto n_random_records = size_t{};
    auto output_file_selected = false;
    try {
        auto desc = po::options_description{"Allowed options"};
        desc.add_options()
                ("help,h", "display help")
                ("file,f", po::value(&file_path_string), "choose file as data source")
                ("random,r", po::value(&n_random_records), "generate N random records")
                ("user,u", "user input for records generation")
                ("verbose,v", "prints data after every stage of sorting")
                ("debug,d", "prints debug info (may affect I/O counters")
                ("output,o", po::value(&output_file_path_string), "set output file")
                ("measure,m", "make measurements");
        auto vm = po::variables_map{};
        po::store(po::parse_command_line(argc, argv, desc), vm);
        po::notify(vm);
        if (vm.count("help")) {
            std::cout << desc << '\n';
            return 0;
        }
        if (vm.count("output")) output_file_selected = true;
        if (vm.count("verbose")) Config::verbose = true;
        if (vm.count("debug")) Config::debug = true;
        if (vm.count("measure")) action = Action::MEASURE;
        else if (vm.count("file")) {
            file_path = fs::path{file_path_string};
            if (!fs::is_regular_file(file_path))
                throw po::error("Specified file does not exists or is not a file: " + fs::absolute(file_path).string());
            action = Action::SORT_CSV_FILE;
        } else if (vm.count("random")) action = Action::SORT_RANDOM_DATA;
        else if (vm.count("user")) action = Action::SORT_USER_INPUT_DATA;

        if (action == Action::NOT_SELECTED) {
            cout << desc << '\n';
            return 0;
        }
    } catch (po::error &e) {
        cerr << "Error parsing program arguments\n" << e.what() << endl;
        return -1;
    }
    // Create buffer
    auto buf = (output_file_selected) ?
               Buffer_t(output_file_path_string.c_str(), Buffer_t::Mode::WRITE) :
               Buffer_t(Buffer_t::Mode::WRITE);

    // Fill the buffer
    switch (action) {
        case Action::SORT_CSV_FILE:
            verbose([&] { cout << "Reading data from csv file: " << fs::absolute(file_path) << endl; });
            RecordsGenerator::from_csv_file(buf, file_path);
            verbose([] { cout << "OK" << endl; });
            break;
        case Action::SORT_RANDOM_DATA:
            verbose([&] { cout << "Generating " << n_random_records << " random records ..." << endl; });
            RecordsGenerator::random(n_random_records, buf);
            verbose([] { cout << "OK" << endl; });
            break;
        case Action::SORT_USER_INPUT_DATA:
            cout << "Put grade1 grade2 grade3, ctrl+d to end\n" << endl;
            RecordsGenerator::from_keyboard(buf);
            break;
        case Action::MEASURE:
            verbose([] { cout << "Making measurements ..."; });
            break;
        case Action::NOT_SELECTED:
            break;
    }

    verbose([&] {
        cout << "Sorting (natural merge sort 2+1) ...\n" << endl << "Before sort:\n";
        buf.print_all_records();
    });

    // Sort buffer
    auto[iters, reads, writes, rec_reads, rec_writes] = Sorter::natural_merge_sort_2_1(buf);

    verbose([&] {
        cout << "After sort:\n";
        buf.print_all_records();
        cout << "OK" << endl;
    });


    // Print summary
    Terminal::set_color(Terminal::Color::FG_GREEN);
    cout << "Summary:\nIterations: " << iters << "\t\tDisk reads: " << reads << "\t\tDisk writes: " << writes
         << "\t\tTotal disk I/O: " << reads + writes
         << "\nRecords reads: " << rec_reads << "\t\tRecords writes: " << rec_writes
         << "\t\tTotal records R/W: " << rec_reads + rec_writes;
    Terminal::set_color(Terminal::Color::FG_DEFAULT);
    return 0;
}
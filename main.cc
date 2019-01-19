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

using Buffer_t = Buffer<Config::BUFFER_SIZE>;
using std::cout, std::cerr,std::endl;
namespace po = boost::program_options;
namespace fs = std::filesystem;


enum class Action {
    NOT_SELECTED,
    SORT_INPUT_FILE,
    SORT_RANDOM_DATA,
    SORT_USER_INPUT_DATA,
    MEASURE,
    GENERATE_TEST_FILES,
    PRINT_FILE
};


auto main(int argc, char **argv) -> int{

    // Parse program arguments
    Action action = Action::NOT_SELECTED;
    std::string file_path_string;
    fs::path file_path;
    std::string output_file_path_string;
    size_t n_random_records;
    bool output_file_selected = false;
    try {
        auto desc = po::options_description{"Allowed options"};
        desc.add_options()
                ("help,h", "display this help")
                ("output,o", po::value(&output_file_path_string), "save sorted records in file")
                ("file,f", po::value(&file_path_string), "sort given file")
                ("random,r", po::value(&n_random_records), "sort N random generated records")
                ("user,u", "sort user input records")
                ("verbose,v", "prints verbose messages about program execution")
                ("debug,d", "print records every iteration and other debug info (may affect I/O counters)")
                ("measure,m", "make defined measurements")
                ("generate,g", "generate test files")
                ("print,p", po::value(&file_path_string), "print given file at standard output");
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

        if (vm.count("generate")) action = Action::GENERATE_TEST_FILES;
        else if (vm.count("measure")) action = Action::MEASURE;
        else if (vm.count("file")) {
            file_path = fs::path{file_path_string};
            if (!fs::is_regular_file(file_path))
                throw po::error("Specified file does not exist or is not a file: " + fs::absolute(file_path).string());
            action = Action::SORT_INPUT_FILE;
        } else if (vm.count("print")) {
            file_path = fs::path{file_path_string};
            if (!fs::is_regular_file(file_path))
                throw po::error("Specified file does not exist or is not a file: " + fs::absolute(file_path).string());
            action = Action::PRINT_FILE;
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


    verbose([] { cout << "Buffer size: " << std::to_string(Config::BUFFER_SIZE) << " bytes\n"; });
    verbose([&] {
        if (output_file_selected)
            cout << "Output file: " << fs::absolute(output_file_path_string).string() << '\n';
    });


    // Create buffer
    auto buf = (output_file_selected) ?
               Buffer_t(output_file_path_string.c_str(), Buffer_t::Mode::WRITE) :
               Buffer_t(Buffer_t::Mode::WRITE);

    switch (action) {
        case Action::SORT_INPUT_FILE:
            verbose([&] { cout << "Reading data from file: " << fs::absolute(file_path) << endl; });
            buf.load_from_file(file_path);
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
            cout << "Making measurements ...\n";
            Measurements::io();
            return 0;
            break;
        case Action::GENERATE_TEST_FILES:
            verbose([] { cout << "Generating test files ...\n"; });
            RecordsGenerator::generate_test_files();
            return 0;
        case Action::PRINT_FILE:
            verbose([&] { cout << "Printing file " << fs::absolute(file_path).string(); });
            buf.load_from_file(file_path);
            buf.print_all_records(Buffer_t::PrintMode::FULL);
            return 0;
        case Action::NOT_SELECTED:
            break;
    }

    verbose([&] {
        cout << "Before sort:\n";
        buf.print_all_records();
        cout << "Sorting (natural merge sort 2+1) ...\n";
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
    cout << "Summary:\nIterations: " << iters << "\nDisk reads: " << reads << "\nDisk writes: " << writes
         << "\nTotal disk I/O: " << reads + writes
         << "\nRecords reads: " << rec_reads << "\nRecords writes: " << rec_writes
         << "\nTotal records R/W: " << rec_reads + rec_writes << endl;
    Terminal::set_color(Terminal::Color::FG_DEFAULT);
    return 0;
}
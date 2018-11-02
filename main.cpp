#include <iostream>
#include <bitset>
#include "record.hh"
#include "buffer.hh"
#include "records_generator.hh"
#include "sorter.hh"
/*
 * WARNING: Program is currently valid only for LE machines
 */
using Buffer_t = Buffer<4096 * 64>;
int main() {
    using std::cout;
    using std::endl;
    Buffer_t buf("./buf01", Buffer_t::Mode::WRITE);
    RecordsGenerator::Random(/*67'108'864*/ 7, buf);
    RecordsGenerator::ReadFromKeyboard(buf);
    Sorter::natural_merge_sort_2_1(buf);
    return 0;
}
#include <iostream>
#include <bitset>
#include "record.hh"
#include "buffer.hh"
#include "records_generator.hh"
#include "sorter.hh"
/*
 * WARNING: Program is currently valid only for LE machines
 */
using buffer_t = Buffer<4096 * 64>;
int main() {
    using std::cout;
    using std::endl;
    buffer_t buf("/home/kamil/Desktop/buf01", buffer_t::Mode::WRITE);
    RecordsGenerator::Random(/*67'108'864*/ 5, buf);
    Sorter::natural_merge_sort_2_1(buf);
    return 0;
}
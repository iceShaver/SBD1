#include <iostream>
#include <bitset>
#include "record.hh"
#include "buffer.hh"
#include "records_generator.hh"
/*
 * WARNING: Program is currently valid only for LE machines
 */
using buffer_t = Buffer<4096 * 64>;
int main() {
    using std::cout;
    using std::endl;
    buffer_t buf("/home/kamil/Desktop/buf01", buffer_t::Mode::WRITE);
    RecordsGenerator::Random(/*67'108'864*/ 3, buf);
    cout << buf;
    buf.PrintAllRecords();
    cout << endl;
    cout << buf << endl;
    buf.PrintAllRecords();
    cout << endl;
    cout << buf << endl;
    buf.PrintAllRecords();
    buf.WriteRecord({123, 25, 36, 21});
    cout << buf << endl;
    cout << endl;
    buf.PrintAllRecords();
    cout << buf << endl;
    return 0;
}
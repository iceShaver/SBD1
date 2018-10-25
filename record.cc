//
// Created by kamil on 19.10.18.
//

#include "record.hh"
#include <exception>
#include <stdexcept>
#include <cmath>

uint Record::GetGrade(int gradeNumber) const {
    if (gradeNumber <= GRADES_NUMBER && gradeNumber > 0) {
        return (uint) (((uint8_t) (((uint8_t) data) << (2 * gradeNumber))) >> 6) + 2;
    }
    throw std::invalid_argument("gradeNumber has to be <1, 3>");
}
Record::Record(data_t data) : data(data) { calcAvg(); }
std::array<uint8_t, sizeof(Record::data_t)> Record::ToBytes() const {
    auto result = std::array<uint8_t, sizeof(data_t)>();
    *((data_t *) result.data()) = data;
    return result;
}
Record::Record(uint32_t student_id, uint8_t grade1, uint8_t grade2, uint8_t grade3) {
    if (student_id >= std::pow(2, 26)) { throw std::invalid_argument("student id over 2^26 (67108864)"); }
    if (grade1 > 5 || grade2 > 5 || grade3 > 5 || grade1 < 2 || grade2 < 2 || grade3 < 2) {
        throw std::invalid_argument("invalid grade, allowed: 2,3,4,5");
    }
    grade1 -= 2;
    grade2 -= 2;
    grade3 -= 2;
    data = grade3 | (grade2 << 2) | (grade1 << 4) | (student_id << 6);
    calcAvg();
}
void Record::calcAvg() {
    avg = 0;
    for (int i = 1; i <= GRADES_NUMBER; ++i) { avg += GetGrade(i); }
    avg /= (double) GRADES_NUMBER;
}
std::ostream &operator<<(std::ostream &os, const Record &record) {
    const char * tabs = "\t\t\t";
    return os << record.GetStudentId() << tabs << record.GetGrade(2) << tabs<< record.GetGrade(2) << tabs
              << record.GetGrade(3) << tabs << record.GetAvg();
}

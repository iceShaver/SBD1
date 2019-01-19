//
// Created by kamil on 19.10.18.
//

#include "record.hh"
#include <exception>
#include <stdexcept>
#include <string>
#include <cmath>
#include <iomanip>

auto Record::get_grade(int gradeNumber) const -> uint8_t {
    if (gradeNumber <= GRADES_NUMBER && gradeNumber > 0)
        return static_cast<uint8_t>(data >> (3 - gradeNumber) * 8);
    throw std::invalid_argument("gradeNumber has to be <1, 3>");
}

Record::Record(data_t data) : data(data) { calc_avg(); }

Record::Record(uint64_t student_id, uint8_t grade1, uint8_t grade2, uint8_t grade3) {
    if (student_id >= std::pow(2, 40))
        throw std::invalid_argument("student id over " + std::to_string(std::pow(2, 40)));
    if (grade1 > GRADE_MAX || grade2 > GRADE_MAX || grade3 > GRADE_MAX ||
        grade1 < GRADE_MIN || grade2 < GRADE_MIN || grade3 < GRADE_MIN)
        throw std::invalid_argument("invalid grade used to instatiate record");
    data = grade3 | grade2 << 8 | grade1 << 16 | student_id << 24;
    calc_avg();
}

Record::Record(uint8_t grade1, uint8_t grade2, uint8_t grade3) : Record(record_id_counter++, grade1, grade2, grade3) {}

auto Record::to_bytes() const -> std::array<uint8_t, sizeof(data_t)> {
    auto result = std::array<uint8_t, sizeof(data_t)>();
    *reinterpret_cast<data_t *>(result.data()) = data;
    return result;
}

auto Record::calc_avg() -> void {
    avg = 0;
    for (int i = 1; i <= GRADES_NUMBER; ++i) avg += get_grade(i);
    avg /= (double) GRADES_NUMBER;
}

auto operator<<(std::ostream &os, const Record &record) -> std::ostream & {
    auto col_width = 20;
    return os << std::setw(col_width) << record.get_student_id()
              << std::setw(col_width) << +record.get_grade(1)
              << std::setw(col_width) << +record.get_grade(2)
              << std::setw(col_width) << +record.get_grade(3)
              << std::setw(col_width) << std::setprecision(2)
              << std::fixed << record.get_avg();
}

auto Record::get_student_id() const -> uint64_t { return data >> 24; }

auto Record::random() -> Record {
    return Record{static_cast<uint8_t>(uid(gen)), static_cast<uint8_t>(uid(gen)), static_cast<uint8_t>(uid(gen))};
}

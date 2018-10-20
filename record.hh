//
// Created by kamil on 19.10.18.
//

#ifndef SBD_1_RECORD_HH
#define SBD_1_RECORD_HH


#include <cstdint>
#include <array>

constexpr const int GRADES_NUMBER = 3;

/*
 * Record contains:
 * studentID, three grades (1, 2, 3)
 * 26 bits,   6 bit
 * students: 67_108_864
 * grades: 2, 3, 4, 5
 */
class Record final {
public:
    using data_t = uint32_t;
    //    Record() = delete;
//    Record(Record const &) = delete;
//    Record(Record &&) = delete;
//    Record &operator=(Record const &) = delete;
//    Record &operator=(Record &&) = delete;
    explicit Record(data_t data);
    explicit Record(std::array<char, sizeof(data_t)> const &bytes) : Record(*(Record::data_t *) bytes.data()) {}
    Record(uint32_t student_id, u_char grade1, u_char grade2, u_char grade3);
    ~Record() = default;


    auto GetStudentId() const { return data >> 6; };
    uint GetGrade(int gradeNumber) const;
    auto GetAvg() const { return avg; }
    std::array<char, sizeof(data_t)> ToBytes() const;
private:
    void calcAvg();
    data_t data;
    float avg;
};

#endif //SBD_1_RECORD_HH

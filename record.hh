//
// Created by kamil on 19.10.18.
//

#ifndef SBD_1_RECORD_HH
#define SBD_1_RECORD_HH


#include <cstdint>
#include <array>
#include <ostream>


constexpr const int GRADES_NUMBER = 3;

/*
 * Record contains (uint64_t):
 * 40 bits student id, 8+8+8 bits for grades
 * grade: 1-100 points
 */
class Record final {
public:
    using data_t = uint64_t;
    static constexpr const auto GRADE_MAX = 100u;
    static constexpr const auto GRADE_MIN = 0u;
    //    Record() = delete;
//    Record(Record const &) = delete;
//    Record(Record &&) = delete;
//    Record &operator=(Record const &) = delete;
//    Record &operator=(Record &&) = delete;
    explicit Record(data_t data);
    Record(uint64_t student_id, uint8_t grade1, uint8_t grade2, uint8_t grade3);
    ~Record() = default;


    uint64_t GetStudentId() const;
    uint8_t GetGrade(int gradeNumber) const;
    double GetAvg() const { return avg; }
    std::array<uint8_t, sizeof(data_t)> ToBytes() const;
    friend std::ostream &operator<<(std::ostream &os, const Record &record);
private:
    void calcAvg();
    data_t data;
    float avg;
};

#endif //SBD_1_RECORD_HH

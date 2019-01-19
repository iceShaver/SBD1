//
// Created by kamil on 19.10.18.
//

#ifndef SBD_1_RECORD_HH
#define SBD_1_RECORD_HH


#include <cstdint>
#include <array>
#include <ostream>
#include <random>


constexpr const int GRADES_NUMBER = 3;

/*
 * Record contains (uint64_t):
 * 40 bits student id, 8+8+8 bits for grades
 * grade: 1-100 points
 */
class Record final {
public:
    using data_t = uint64_t;
    static constexpr auto const GRADE_MAX = 100u;
    static constexpr auto const GRADE_MIN = 0u;

    Record() = delete;
    Record(Record const &) = default;
    Record(Record &&) = default;
    Record &operator=(Record const &) = default;
    Record &operator=(Record &&) = default;

    explicit Record(data_t data);
    Record(uint8_t grade1, uint8_t grade2, uint8_t grade3);
    ~Record() = default;


    auto get_student_id() const -> uint64_t;
    auto get_grade(int gradeNumber) const -> uint8_t;
    auto get_avg() const -> double { return avg; }
    auto to_bytes() const -> std::array<uint8_t, sizeof(data_t)>;

    auto operator<(Record const &rhs) const -> bool { return avg < rhs.avg; }
    auto operator>(Record const &rhs) const -> bool { return rhs < *this; }
    auto operator<=(Record const &rhs) const -> bool { return !(rhs < *this); }
    auto operator>=(Record const &rhs) const -> bool { return !(*this < rhs); }
    static auto random() -> Record;
private:
    Record(uint64_t student_id, uint8_t grade1, uint8_t grade2, uint8_t grade3);
    auto calc_avg() -> void;

    friend auto operator<<(std::ostream &os, const Record &record) -> std::ostream &;

    data_t data;
    float avg;

    inline static uint64_t record_id_counter = 0;
    inline static std::random_device rd{};
    inline static std::mt19937_64 gen = std::mt19937_64{rd()};
    inline static std::uniform_int_distribution<> uid{GRADE_MIN, GRADE_MAX};
};

#endif //SBD_1_RECORD_HH

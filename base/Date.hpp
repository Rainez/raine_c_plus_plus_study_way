#ifndef RAINE_DATE_H
#define RAINE_DATE_H
#include "../copyable.h"
#include <utility>
#include <string>
struct tm;

namespace raine
{
class Date : public raine::copyable
{
public:
    struct YearMonthDay
    {
        int year;
        int month;
        int day;
    };
    static const int kDayPerWeek = 7;
    static const int kJulianDayOf1970_01_01;
    
    Date():julianDayNumber_(0) { }
    Date(int year, int month, int day);
    explicit Date(int julianDayNum) { }
    explicit Date(const struct tm&);

    void swap(Date& that)
    {
        std::swap(julianDayNumber_, that.julianDayNumber_);
    }

    bool valid() const { return julianDayNumber_ > 0; }

    std::string toIsoString() const;
    struct YearMonthDay yearMonthDay() const;
    int year() const { return yearMonthDay().year; }
    int month() const { return yearMonthDay().month; }
    int day() const { return yearMonthDay().day; }
    
    int weekDay() const {
        return (julianDayNumber_+1)%kDayPerWeek;
    }

    int julianDayNumber() const {
        return julianDayNumber_;
    }
private:
    int julianDayNumber_;
};

inline bool operator<(Date x, Date y) {
    return x.julianDayNumber() < y.julianDayNumber();
}

inline bool operator==(Date x, Date y) 
{
    return x.julianDayNumber() == y.julianDayNumber();
}
} // namespace raine
#endif
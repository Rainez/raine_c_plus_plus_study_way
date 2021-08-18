#ifndef RAINE_TIMESTAMP_H
#define RAINE_TIMESTAMP_H
#include "copyable.h"
#include <boost/operators.hpp>
#include <string>
namespace raine
{
// 时间戳的类;单位ms;时间戳是不可变的类；用传递值的方式传递
class Timestamp: public copyable,
                 public boost::equality_comparable<Timestamp>,
                 public boost::less_than_comparable<Timestamp> {
public:
    explicit Timestamp(int64_t microSecondsSinceEpoch = 0):microSecondsSinceEpoch_(microSecondsSinceEpoch) {}
    
    void swap(Timestamp& that) {
        std::swap(microSecondsSinceEpoch_, that.microSecondsSinceEpoch_);
    }

    std::string toString() const;
    std::string toFormattedString(bool showMicroseconds = true) const;

    bool valid() const { return microSecondsSinceEpoch_ > 0; }
    int64_t microSecondsSinceEpoch() const { return microSecondsSinceEpoch_; }
    time_t secondsSinceEpoch() const { return static_cast<time_t>(microSecondsSinceEpoch_ / kMicroSecondsPerSeconds); }

    static Timestamp now();
    static Timestamp invalid() { return Timestamp(); }
    static Timestamp fromUnixTime(time_t t, int microseconds = 0) {
        return Timestamp(static_cast<int64_t>(t)*kMicroSecondsPerSeconds+microseconds);
    }
    constexpr static int kMicroSecondsPerSeconds = 1000*1000;
private:
    int64_t microSecondsSinceEpoch_;
};

inline bool operator<(Timestamp lhs, Timestamp rhs) {
    return lhs.microSecondsSinceEpoch() < rhs.microSecondsSinceEpoch();
}

inline bool operator==(Timestamp lhs, Timestamp rhs) {
    return lhs.microSecondsSinceEpoch() == rhs.microSecondsSinceEpoch();
}

inline double timeDifference(Timestamp high, Timestamp low) {
    int64_t diff = high.microSecondsSinceEpoch() - low.microSecondsSinceEpoch();
    return static_cast<double>(diff)/Timestamp::kMicroSecondsPerSeconds;
}

inline Timestamp addTime(Timestamp timestamp, double seconds) {
    return Timestamp(timestamp.microSecondsSinceEpoch()+static_cast<int64_t>(seconds*Timestamp::kMicroSecondsPerSeconds));
}
} // namespace raine
#endif
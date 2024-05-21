#pragma once

#include <chrono>
#include <thread>
#include <atomic>
#include <sstream>
#include <ctime>
#include <iomanip>
#include <string>

namespace TimeStamp{

class TimeStamp{
public:
    TimeStamp();
    //拷贝有const限定符
    //移动是右值类型的参数
    TimeStamp(const TimeStamp& timeStamp);
    TimeStamp(TimeStamp&& timeStamp);
    TimeStamp& operator=(const TimeStamp&);
    TimeStamp& operator=(TimeStamp&&);

    ~TimeStamp();

    std::chrono::system_clock::time_point
    getTimeStamp() const;

    void setTimeStamp(std::chrono::system_clock::time_point timePoint);

    //必须分两个版本，标准库两种都会用到，const引用参数和非const引用参数
    //这样的话，就必须有两个函数，即函数签名是否有const限定符
    inline bool operator <(const TimeStamp other) const{
        return this->timePoint.load() < other.timePoint.load();
    }
    inline bool operator <(TimeStamp other) {
        return this->timePoint.load() < other.timePoint.load();
    }

    inline bool operator ==(const TimeStamp other) const{
        return this->timePoint.load() == other.timePoint.load();
    }

    static TimeStamp addTime(uint64_t timeToAdd);
    static std::string getCurrentTime();

public:
    static const int kMicroSecondsPerSecond = 1000 * 1000;
    static const int kMilliSecondsPerSecond = 1000;

private:
    std::atomic<std::chrono::system_clock::time_point> timePoint;
};

unsigned int DiffTimePoint(const TimeStamp& now, const TimeStamp& next);

}
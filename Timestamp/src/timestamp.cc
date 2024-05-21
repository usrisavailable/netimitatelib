#include "timestamp/timestamp.hpp"

namespace TimeStamp{

    TimeStamp::TimeStamp()
    {
        this->timePoint.store(std::chrono::system_clock::now());
    }
    TimeStamp::TimeStamp(const TimeStamp& timeStamp)
    {
        this->timePoint.store(timeStamp.getTimeStamp());
    }
    TimeStamp::TimeStamp(TimeStamp&& timeStamp)
    {
        this->timePoint.store(timeStamp.getTimeStamp());
    }
    TimeStamp& TimeStamp::operator=(TimeStamp&& timeStamp) {
        if (this == &timeStamp)
            return *this;
        this->timePoint.store(timeStamp.getTimeStamp());
        return *this;
    }
    TimeStamp& TimeStamp::operator=(const TimeStamp& timeStamp) {
        if (this == &timeStamp)
            return *this;
        this->timePoint.store(timeStamp.getTimeStamp());
        return *this;
    }
    TimeStamp::~TimeStamp()
    {
        //do nothing
    }

    std::chrono::system_clock::time_point
    TimeStamp::getTimeStamp() const
    {
        return this->timePoint.load();
    }

    void TimeStamp::setTimeStamp(std::chrono::system_clock::time_point timePoint)
    {
        this->timePoint.store(timePoint);
    }

    std::string TimeStamp::getCurrentTime()
    {
        std::time_t currentTime;
        currentTime = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
        std::ostringstream oss;
        std::tm* localTime = std::localtime(&currentTime);
        oss << std::put_time(localTime, "%Y-%m-%d %H:%M:%S");
        return oss.str();
    }

    TimeStamp TimeStamp::addTime(uint64_t timeToAdd)
    {
        //factory function
        //无论如何，不应该返回临时对象的引用
        //因此，返回类型不能设置为引用类型
        TimeStamp timeStamp;
        //std::chrono::microseconds interval(timeToAdd);
        std::chrono::milliseconds interval(timeToAdd);
        timeStamp.setTimeStamp(timeStamp.getTimeStamp() + interval);
        return timeStamp;
    }

    unsigned int DiffTimePoint(const TimeStamp& now, const TimeStamp& next) {
        //毫秒计数，精度到个位数，使用chrono库提供的工具
        std::chrono::milliseconds diff = std::chrono::duration_cast<std::chrono::milliseconds>(next.getTimeStamp() - now.getTimeStamp());
        unsigned int ret = diff.count();
        return ret;
    }

}
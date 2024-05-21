#pragma once

#include <memory>
#include <functional>
#include <map>
#include <set>
#include <sys/timerfd.h>
#include <vector>
#include <utility>
#include <mutex>
#include <time.h>

#include "channel/channel.hpp"
#include "timestamp/timestamp.hpp"
#include "eventloop/eventloop.hpp"

namespace Eventloop{
    class Eventloop;
}
namespace Channel{
    class Channel;
}
namespace Timerqueue{

using TimerCallbackFunc = std::function<void()>;

//自定义类型，虽然系统计时对象也是timer

class Timer{
public:
    Timer(TimeStamp::TimeStamp& expire,
            TimerCallbackFunc& cb,
            uint64_t interval);
    Timer(Timer&) = default;
    Timer(Timer&&) = default;
    Timer& operator=(Timer&) = default;
    Timer& operator=(Timer&&) = default;

    ~Timer();

    /**
     * 重载比较操作符
     * 直接对指针对象进行比较，一个64位的无符号整数值
     * set存储自定对象，需要提供比较方法，
     * 我们这里使用了pair类型处理比较问题
    */
    inline bool operator < (Timer& otherTimer) {
        return this < &otherTimer;
    }
    inline bool operator == (Timer& otherTimer) {
        return this == &otherTimer;
    }

    TimeStamp::TimeStamp& getExpire() {
        return this->expiration;
    }

    void run(){
        cbRead(); 
    }
    bool repeat() {
        return isRepeat;
    }

    /**
     * 重新启动timer
     * 问题点：要不要在函数内部完成入队列
     * @para nonne
     * @ret none
    */
    void restart();
    /**
     * 拷贝一份自身的资源，表面模仿enable_shared_from_this
     * 问题：使用了同一份资源去初始化不同的unique_ptr对象
     * @ret unique_ptr<Timer>
    */
    std::unique_ptr<Timer> getCopy() {
        return std::make_unique<Timer>(*this);
    }
 
private:
    TimeStamp::TimeStamp expiration;
    TimerCallbackFunc cbRead;
    uint64_t interval;
    bool isRepeat;
};

struct TimerCompare {
    bool operator() (const std::unique_ptr<Timer>& p1, const std::unique_ptr<Timer>& p2) {
        return *p1 < *p2;
    }
};

//how to traversal the timerlist
//how choose the key
using Entry = std::pair<TimeStamp::TimeStamp, Timer* >;

class Timerqueue{
public:
    Timerqueue(std::shared_ptr< Eventloop::Eventloop > eventloop);
    Timerqueue(Timerqueue&) = delete;
    Timerqueue(Timerqueue&&) = delete;
    Timerqueue& operator= (const Timerqueue& timer) {
        if (this == &timer)
            return *this;
        this->loop = timer.loop;
        return *this;
    }
    ~Timerqueue();
    void Init();
    /**
     *this thread exposd for other class
     *@para 传递对象引用会将对象所有权转移给其他函数，unique_ptr对象在值传递过程中只会有所有权的转移，行为和shared_ptr对象不一样，没有引用计数概念。本函数需要拿到timer对象的所有权，以便在后面自动销毁不在需要的timer对象
     *@ret 后面有需要在更改API设计
    */
    void addTimer(TimeStamp::TimeStamp, TimerCallbackFunc, double);
private:
    /**
     * timer插入的实际执行体
     * @para timer对象的所有权也同样需要像下一个调用函数转移
     * 传递对象引用会将对象所有权转移给其他函数，unique_ptr对象在值传递过程中只会有所有权的转移，行为和shared_ptr对象不一样，没有引用计数概念。本函数需要拿到timer对象的所有权，以便在后面自动销毁不在需要的timer对象
     * @ret 假设所有动作都是执行成功的
    */
    void addTimerInLoop(Timer* timer);
    /**
     * create a file descriptor referenced to a timerfd
     * @para 只能是raw pointer，因为unique_ptr无法被拷贝
     * 接收raw pointer之后再交由unique_ptr处理
     * @ret a fd referenced to the new created timer object(system call)
    */
    int createTimerfd();
    /**
     * 系统timerfd有事件触发（到期）后的处理
     * @para none
     * @ret none
    */
    void handleTimerfd();
    /**
     * 在每次有新的计时器如队列后，arm or disarm the timer
     * @para none
     * @ret none
    */
    /**
     * 重启定时器，以最新的时间为标准
    */
    void resetTimerfd(bool earliest);
    /**
     * 计时器入队列
     * 入队后，通知resetTimerfd function
     * 
     * @para 构造timerList的一个元素
     * @ret none
    */
    void insert(TimeStamp::TimeStamp timeStamp, Timer* timer);
    /**
     * 每次expire后，重置expire中需要重新启动的timer
     * @para expiredList 过期的timer
    */
    void reset(std::vector< Entry >& expireList);
    /**
     * after system deliver expiration, process all timer
    */
    void handleExpireTimer(std::vector< Entry>& expireList);

private:
    std::shared_ptr< Eventloop::Eventloop > loop;
    std::shared_ptr<Channel::Channel> timerfdChannel;
    std::set< Entry > timerList;    //set是唯一key的集合
    std::mutex timerMutex;  //multi-thread access addTimer() function
};

static bool TimerfdGettime(int fd);

}
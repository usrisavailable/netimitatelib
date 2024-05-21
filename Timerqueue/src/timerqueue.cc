#include "timerqueue/timerqueue.hpp"

namespace Timerqueue{
    //class Timer
    Timer::Timer(TimeStamp::TimeStamp& expire,
            TimerCallbackFunc& cb,
            uint64_t interval)
        :expiration(expire), cbRead(cb), interval(interval)
    {
        isRepeat = false;
        if (interval > 0)
            isRepeat = true;
    }

    Timer::~Timer() {}

    void Timer::restart(){
        expiration.setTimeStamp(
            TimeStamp::TimeStamp::addTime(interval).getTimeStamp());
    }

    //class Timerqueue
    Timerqueue::Timerqueue(std::shared_ptr< Eventloop::Eventloop > eventloop)
        :loop(eventloop)
    {}
    void Timerqueue::Init() {
        //timerfd initialization
        int fd = createTimerfd();
        this->timerfdChannel = std::make_shared < Channel::Channel >(this->loop->GetObject(), fd);
        this->timerfdChannel->SetReadingCallBack(std::bind(&Timerqueue::handleTimerfd, this));
        this->timerfdChannel->EnableReading();

        //当前定时器策略是这样的
        //当前最近将过期的事件，就是定时器下次要触发的时机
        //timerfd的触发时机，目前考虑的有两个
        //一个是有新的事件入队列，二是有到期后队列中仍有未处理事件

    }
   
    Timerqueue::~Timerqueue() {
        //actually, nothing need to do
        timerList.clear();
    }

    int Timerqueue::createTimerfd() {
        int ret = ::timerfd_create(CLOCK_MONOTONIC, TFD_CLOEXEC | TFD_NONBLOCK);
        //想不到啥更好的方式
        assert(ret != -1);
        return ret;
    }

    void Timerqueue::handleTimerfd() {
        //1、处理system timer expire
        u_int64_t one = 1;
        int ret = ::read(timerfdChannel->GetFd(), &one, sizeof one);
        assert(ret == 8);

        //2、处理队列中的expire task
        //寻找可用的到期区间
        //处理方法完全参考其他代码，利用指针自身的值进行比较
        std::vector< Entry > expiredTimerList;
        //这段比较代码应该是没问题的，一个指针对象占据8个字节的存储空间
        //UINTPTR_MAX这个就是系统定义的的指针对象的最大值
        //解决多个定时器到期时间一样的重复问题，否则就不能使用set数据结构
        Entry sentry = std::make_pair(TimeStamp::TimeStamp{}, reinterpret_cast<Timer*>(UINTPTR_MAX));
        std::set<Entry>::iterator iter = timerList.lower_bound(sentry);
        //第一个版本，使用了copy函数和make_move_iterator
        //使用move函数效果一样
        //set的遍历会返回const unique_ptr<>&类型，是一个常左值引用
        //它无法被转换为unique_ptr<>&&类型
        //所以back_inserter最终会调用unique_ptr<>(unique_ptr<>&) = delete;
        /* std::copy(std::make_move_iterator(timerList.begin()), 
                std::make_move_iterator(iter), 
                std::back_inserter(expiredTimerList)); */
        //为了解决这个问题，可以使用C++17的merge函数，它可以直接接管set中的元素
        //其他方法就是复制一份新的出来，再删除旧的
        //当元素存到vector中是，可以安全的转移进入set，因为vector返回的引用类型没有cv限定符
        //事实上，这样做依旧不行，还是没办法避免改动set中的元素
        /* for (auto it = timerList.begin(); it != iter; it++)
        {
            expiredTimerList.emplace_back(it->second->getCopy());
            const_cast<std::unique_ptr<Timer>&>(it->second).release();
        } */
        //那么只好用原始指针了，而且必须考虑内存管理
        {
            std::lock_guard<std::mutex> lock(this->timerMutex);
            std::copy(timerList.begin(), iter, 
                    std::back_inserter(expiredTimerList));
            timerList.erase(timerList.begin(), iter);
        }
        handleExpireTimer(expiredTimerList);
        return;
    }

    void Timerqueue::handleExpireTimer(std::vector< Entry >& expireList) {
        for(auto& val : expireList) {
            val.second->run();
        }
        reset(expireList);
        return;
    }

    void Timerqueue::reset(std::vector< Entry >& expireList) {
        //所有过期且已处理的timer，需要重置
        //最后重置系统timer fd
        for(auto& val : expireList) {
            if (val.second->repeat()) {
                val.second->restart();
                this->insert(val.second->getExpire(), val.second);
            }
            else {
                delete val.second;
            }
        }
        //有必要判断是否要重启定时器
        //因为定时器任务如对列发生在处理到期任务中，很可能会不会重设timerfd
        //我觉得是判断条件不合理，但是不会改
        if (!timerList.empty() && TimerfdGettime(this->timerfdChannel->GetFd()))
            resetTimerfd(true);
        return;
    }
    void Timerqueue::insert(TimeStamp::TimeStamp timeStamp, Timer* timer) {
        //将所有timerList.emplace的调用全部放到这里
        //定时器队列是开放给全部线程的，必须进行保护
        //入队之后，下一步就是处理函数resetTimerfd
        bool earliest = false;
        std::lock_guard<std::mutex> lock(timerMutex);
        std::pair<std::set<Entry>::iterator, bool> ret = this->timerList.emplace(timeStamp, timer);
        //具体的定时器事件入队操作，只要拿到loop对象的都可以设置定时器任务
        auto iter = timerList.cbegin();
        //Loginfo::Loginfo::infoPut(timerList.size());
        if (timer == iter->second)
            resetTimerfd(earliest); 
        return;
    }
    void Timerqueue::resetTimerfd(bool earliest) {
        //muduo选择即将到期的时间作为下次触发时间
        //我选择使用固定时间触发，API也要同时更改
        //固定时间触发，不合理啊
        //定时器事件是已排序的，选择时间最近的那个重启timerfd就可以
        struct itimerspec its;
        //its.it_value.tv_sec = 1000 / TimeStamp::TimeStamp::kMilliSecondsPerSecond;
        //its.it_value.tv_nsec = 0;
        //its.it_interval.tv_sec = 1000 / TimeStamp::TimeStamp::kMilliSecondsPerSecond;
        //its.it_interval.tv_sec = 0;
        //its.it_interval.tv_nsec = 0;
        //计算时间差值
        TimeStamp::TimeStamp now;
        TimeStamp::TimeStamp next = this->timerList.begin()->first;
        //向上累计1秒
        unsigned int diff = static_cast<double>(TimeStamp::DiffTimePoint(now, next)) + 1000;
        its.it_value.tv_sec = diff / TimeStamp::TimeStamp::kMilliSecondsPerSecond;
        its.it_value.tv_nsec = 0;
        its.it_interval.tv_nsec = 0;
        its.it_interval.tv_sec = 0;
        int ret = ::timerfd_settime(timerfdChannel->GetFd(), 0, &its, NULL);
        if (ret)
            perror("timerqueue.cc 151");
        return;
    }

    void Timerqueue::addTimer(TimeStamp::TimeStamp timeStamp, TimerCallbackFunc cb, double interval) {
        //定时器任务交由I\0线程去执行，以便所有线程都可以自行添加timer
        //task在线程之间传递（面向对象）
        //std::unique_ptr<Timer> timer = std::make_unique<Timer>(timeStamp, cb, interval);
        Timer *timer = new Timer(timeStamp, cb, interval);
        auto timerCB = std::bind(&Timerqueue::addTimerInLoop, this, timer);
        this->loop->RunInLoop(timerCB);
        //如果改变定时策略的话，此处需要其他代码辅助完成任务
        return;
    }
    
    void Timerqueue::addTimerInLoop(Timer* timer) {
        //std::lock_guard<std::mutex> lock(timerMutex);
        //std::pair<std::set<Entry>::iterator, bool> ret = timerList.emplace(timer->getExpire(), timer);
        this->insert(timer->getExpire(), timer);
        return;
    }

    bool TimerfdGettime(int fd) {
        bool ret = false;
        itimerspec currValue;
        ::timerfd_gettime(fd, &currValue);
        //只需要查看下次到期时间，暂时不考虑interval
        currValue.it_value.tv_sec == 0 && currValue.it_value.tv_nsec == 0 ? ret = true : ret = false;
        return ret;
    }

}
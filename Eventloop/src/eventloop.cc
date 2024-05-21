#include <cassert>
#include <iostream>
#include "eventloop/eventloop.hpp"
#include "timestamp/timestamp.hpp"
#include "loginfo/loginfo.hpp"

namespace Eventloop{

Eventloop::Eventloop():
    tid(std::this_thread::get_id()),
    poll(std::make_shared<Poll::Poll>()),
    isQuit(false),
    isLoop(true),
    callingPendingFunc(false),
    timer(nullptr)
{
    channelList = std::make_shared<ChannelList>(0);
    wakeupFd = ::eventfd(0, EFD_CLOEXEC | EFD_NONBLOCK);
}
Eventloop::~Eventloop()
{
    tid = static_cast<std::thread::id>(0);
}

void Eventloop::Init() {
    //搞这个函数的目的是避免在构造函数中操作this指针对象
    //基本上与loop相关的构造函数都要改
    //setup eventfd
    {
        std::shared_ptr<Channel::Channel> eventChannel = std::make_shared<Channel::Channel>(this->GetObject(), wakeupFd);
        eventChannel->SetReadingCallBack(std::bind(&Eventloop::handleRead, this));
        eventChannel->EnableReading();
    }  
    //setup timer queue
    std::unique_ptr<Timerqueue::Timerqueue> timer = std::make_unique<Timerqueue::Timerqueue>(this->GetObject());
    //对智能指针对象调用move函数，只会使得指针对象所有权发生转移
    //不影响其管理的对象
    this->timer = std::move(timer);
    this->timer->Init();
    return;
}

void Eventloop::loop()
{
     Loginfo::Loginfo::infoPut(std::string("thread"),
                                tid,
                                std::string("is starting"),
                                std::string("polling"));
    while (isLoop) {
        static uint32_t count = 0;
        assertInLoop();
        //polling其实会一直阻塞
        int ret = polling();
        /* Loginfo::Loginfo::infoPut(channelList->size(),
                                std::string("event has reached")); */
        for (auto iter : *(channelList.get()))
        {
            iter->HandleEvent();
        }
        DoPendingFunc();
        //std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        /* Loginfo::Loginfo::infoPut(std::string("thread"),
                                tid,
                                std::string("is loopping"),
                                count,
                                std::string("times")); */
    }
    Loginfo::Loginfo::infoPut(std::string("thread"),
                                tid,
                                std::string("stop loopping"));
    return;
}

void Eventloop::UpdateChannel(std::shared_ptr<Channel::Channel> channel){
    poll->RegistEvennt(channel);
}

void Eventloop::RemoveChannel(const std::shared_ptr<Channel::Channel>& channel) {
    this->poll->RemoveChannel(channel);
}

int Eventloop::polling(){
    channelList->clear();
    return poll->poll(channelList);
}

bool Eventloop::assertInLoop()
{
    if (this->tid == std::this_thread::get_id())
        return true;
    Loginfo::Loginfo::infoPut(std::string{"not I/O thread"});
    exit(1);
    return false;
}

bool Eventloop::IsInloop() {
    if (this->tid == std::this_thread::get_id())
        return true;
    return false;
}

void Eventloop::quit() {
    //no garbage collection, just set flag, for now
    isLoop = false;
    isQuit = true;
    this->WakeUp();
    return;
}

void Eventloop::RunInLoop(const FunctorCallBack &callBack) {
    if (assertInLoop())
        callBack();
    else {
        queInLoop(callBack);
    }
}
void Eventloop::queInLoop(const FunctorCallBack &callBack) {
    std::lock_guard<std::mutex> lock(pendingMutex);
    this->pendingList.push_back(callBack);
    if (!assertInLoop() || callingPendingFunc)
        WakeUp();
    return;
}

void Eventloop::DoPendingFunc() {
    //需要尽可能的缩短临界区，访问和插入操作可能会同时进行
    std::vector<FunctorCallBack> funcList;
    //std::unique_lock<std::mutex> lock;
    {
        std::lock_guard<std::mutex> lock(pendingMutex);
        funcList.swap(pendingList);
    }
    this->callingPendingFunc = true;
    for (const auto &idx : funcList)
        idx();
    this->callingPendingFunc = false;
    return;
}

void Eventloop::WakeUp() {
    unsigned long one = 1;
    int ret = ::write(wakeupFd, &one, sizeof one);
    return;
}
void Eventloop::handleRead() {
    unsigned long one = 1;
    int ret = ::read(wakeupFd, &one, sizeof one);
    return;
}

void Eventloop::RunAfter(const FunctorCallBack &callBack, double timeDelay) {
    TimeStamp::TimeStamp timeStamp = TimeStamp::TimeStamp::addTime(timeDelay);
    this->timer->addTimer(timeStamp, callBack, 0);
    return;
}
void Eventloop::RunInterval(const FunctorCallBack &callBack, double timeDelay, double interval) {
    TimeStamp::TimeStamp timeStamp = TimeStamp::TimeStamp::addTime(timeDelay);
    this->timer->addTimer(timeStamp, callBack, interval);
    return;
}

}
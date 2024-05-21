#pragma once

#include <thread>
#include <channel/channel.hpp>
#include <poll/poll.hpp>
#include <timerqueue/timerqueue.hpp>
#include <memory>
#include <vector>
#include <map>
#include <mutex>
#include <sys/eventfd.h>
#include <signal.h>

namespace Channel{
    class Channel;
}

namespace Poll{
    class Poll;
}

namespace Timerqueue{
    class Timerqueue;
}

namespace Eventloop{
class IgonreSigPIpe {
public:
    IgonreSigPIpe() {
        struct sigaction sa;
        ::sigemptyset(&sa.sa_mask);
        sa.sa_flags = 0;
        sa.sa_handler = SIG_IGN;
        if (sigaction(SIGPIPE, &sa, NULL) == -1)
            exit(1);
    }
    ~IgonreSigPIpe() {}
};

class Eventloop : public std::enable_shared_from_this<Eventloop>{
public:
    using ChannelList = std::vector<std::shared_ptr<Channel::Channel>>;
    using ChannelMap = std::map<uint32_t, std::shared_ptr<Channel::Channel>>;
    using FunctorCallBack = std::function<void()>;
    Eventloop();
    ~Eventloop();
    void Init();
    void loop();
    void UpdateChannel(std::shared_ptr<Channel::Channel> channel);
    void RemoveChannel(const std::shared_ptr<Channel::Channel>& channel);
    int polling();
    void quit();
    std::shared_ptr<Eventloop> GetObject() {
        return shared_from_this();
    }
    //exit
    bool assertInLoop();
    //just judge
    bool IsInloop();
    /**
     * 处理一些必须在I/O线程中完成的task
     * @para 需要处理的task，标准库bind返回的类型有C限定符
    */
    void queInLoop(const FunctorCallBack &callBack);
    void RunInLoop(const FunctorCallBack &callBack);
    /**
     * 统一处理所有正在等待的task
    */
   void DoPendingFunc();
   /**
    * 通知eventfd
   */
    void WakeUp();
    /**
     * 处理eventfd发生事件
    */
    void handleRead();

    //定时器接口有关的方法
    /**
     * 第一个触发一次，第二个触发多次
    */
    void RunAfter(const FunctorCallBack &callBack, double timeDelay);
    void RunInterval(const FunctorCallBack &callBack, double timeDelay, double interval);
private:
    bool isQuit;
    bool isLoop;
    std::thread::id tid;
    std::shared_ptr<ChannelList> channelList;   //存储就绪的channel
    std::shared_ptr<Poll::Poll> poll;
    ChannelMap channelMap;  //管理现存的全部channel，无论是否有效
    bool callingPendingFunc;    //标识正在调用
    std::vector<FunctorCallBack> pendingList;
    std::mutex pendingMutex;
    int wakeupFd;   //使用linux提供的eventfd
    std::unique_ptr<Timerqueue::Timerqueue> timer;
};

}


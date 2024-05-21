#pragma once

#include <functional>
#include <sys/epoll.h>

#include <eventloop/eventloop.hpp>
#include <loginfo/loginfo.hpp>
#include <memory>

namespace Eventloop{
    class Eventloop;
}

namespace Channel {
    
enum kEvents {
    ReadEvent = EPOLLIN,        //read event
    WriteEvent = EPOLLOUT,      //write event
    CloseEvent = EPOLLRDHUP,    //close event
    EdgeNotify = EPOLLET,       //edge trigger
    OnceNotify = EPOLLONESHOT,  //use along with edge trigger
    NoneEvent = 0,
};

class Channel : public std::enable_shared_from_this<Channel>{
public:
    using EventCallBack = std::function<void()>;

    Channel(std::shared_ptr<Eventloop::Eventloop> eventloop, int fd);
    ~Channel();
    /**
     * 获取interested event
    */
    u_int32_t GetEvent()
    {
        return event;
    }
    /**
     * 获取索引，主要是方便POLL快速查找channel对象
    */
    uint32_t GetIndex(){
        return index;
    }
    /**
     * 获取已就绪的事件，POLL结束后调用，写入就绪事件
    */
    void SetEvent(uint32_t revent)
    {
        this->revent = revent;
    }
    /**
     * 获取原始的文件描述符
    */
    int GetFd(){
        return socketFd;
    }
    /**
     * 使channel就绪
    */
    void EnableReading();
    void DisableWriting() {
        this->event = this->event & ~kEvents::WriteEvent;
    }
    void EnableWriting();
    /**
     * 设置用户（类使用者）回调函数
    */
    void SetReadingCallBack(EventCallBack cbRead);
    void SetWritingCallBack(EventCallBack cbWrite);
    void SetCloseCallBack(EventCallBack cbClose);
    //在loop内执行
    void HandleEvent();
    //显而易见
    std::shared_ptr<Channel> GetObject()
    {
        return shared_from_this();
    }
    /**
     * 停止接收事件通知
    */
    void DisableAll();
    bool IsWriting() {
        return this->event & kEvents::WriteEvent;
    }
private:
    /**
     * 启动channel的最后一步，在Enable系成员函数中调用
    */
    void update();
private:    
    //判定的eventloop不为NULL的方法有很多，weak_ptr是一种方法
    std::weak_ptr<Eventloop::Eventloop> eventloop;
    //idex和socketFd在有意义的情况下是同一个值
    int socketFd;
    uint32_t index;
    //redey evennt and interested event
    u_int32_t revent;
    u_int32_t event;
    EventCallBack cbRead;
    EventCallBack cbWrite;
    EventCallBack cbShutdown;
};

}
#include "channel/channel.hpp"
#include "unistd.h"

namespace Channel{

    Channel::Channel(std::shared_ptr<Eventloop::Eventloop> eventloop,
                    int fd)
    {   
        this->eventloop = eventloop;
        this->revent = 0;
        this->socketFd = fd;
        this->index = fd;   //文件描述符作为系统资源，不会出现重复的fd被创建
        this->cbWrite = static_cast<EventCallBack>(0);
        this->cbRead = static_cast<EventCallBack>(0);
        this->cbShutdown = static_cast<EventCallBack>(0);
    }

    Channel::~Channel()
    {
        //挺多余的操作
        this->socketFd = -1;
        this->index = -1;
    }
    void Channel::EnableReading(){
        this->event = kEvents::ReadEvent | kEvents::CloseEvent;
        this->update();
    }
    void Channel::EnableWriting(){
        this->event = this->event | kEvents::WriteEvent;
        this->update();
    }
    void Channel::SetReadingCallBack(EventCallBack cbRead)
    {
        this->cbRead = cbRead;
    }
    void Channel::SetWritingCallBack(EventCallBack cbWrite)
    {
        this->cbWrite = cbWrite;
    }
    void Channel::SetCloseCallBack(EventCallBack cbClose) {
        this->cbShutdown = cbClose;
    }

    void Channel::update()
    {
        if (!eventloop.lock())
            {
                Loginfo::Loginfo::infoPut(std::string{"channel update failed!"});
                return;
            }
        this->eventloop.lock()->UpdateChannel(shared_from_this());
    }
    //这里的判断应该没问题
    void Channel::HandleEvent()
    {
        //啥都不考虑，只要是有断开事件，就先处理断开事件
        //其次，读和写
        //实际上，好多事件是被or在一起的
        if (revent & kEvents::CloseEvent)
            {this->cbShutdown(); return;}
        if (revent & kEvents::ReadEvent)
            {this->cbRead(); return;}
        if (revent & kEvents::WriteEvent)
            {this->cbWrite(); return;}
        return;
    }

    void Channel::DisableAll() {
        this->event = kEvents::NoneEvent;
        this->update();
    }
}
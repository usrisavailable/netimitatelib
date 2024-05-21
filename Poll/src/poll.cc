#include "poll/poll.hpp"

namespace Poll{
    Poll::Poll()
    {
        //do not care error
        this->epfd = epoll_create(5);
        this->channelMap.clear();
    }
    Poll::~Poll()
    {
        if (-1 != epfd)
        ::close(this->epfd);
    }
    void Poll::RegistEvennt(std::shared_ptr<Channel::Channel> channel)
    {
        std::shared_ptr<Channel::Channel> channelElement = channel;
        int index = channel->GetIndex();
        if (channelMap.find(index) == channelMap.end()) {
            //regist new channel
            channelMap.insert({index, channelElement});
            PollEvent pollEvent;
            pollEvent.data.ptr  = channel.get();    //raw pointer
            pollEvent.events = channel->GetEvent();
            ::epoll_ctl(epfd, 
                    EPOLL_CTL_ADD,
                    channel->GetFd(),
                    &pollEvent);
        } else {
            //update the channel finded
            PollEvent pollEvent;
            pollEvent.data.ptr  = channel.get();    //raw pointer
            pollEvent.events = channel->GetEvent();
            epoll_ctl(epfd, 
                    EPOLL_CTL_MOD,
                    channel->GetFd(),
                    &pollEvent);
            //only one operation, delete
            //channelMap.erase(index);
        }
        return;
    }
    void Poll::RemoveChannel(const std::shared_ptr<Channel::Channel>& channel) {
        //需要做错误处理
        ::epoll_ctl(epfd, 
                EPOLL_CTL_DEL,
                channel->GetFd(),
                NULL);
        //没有其他对象持有channel的话，其引用计数就为2，之后会销毁
        channelMap.erase(channel->GetIndex());
    }
    //wrong method
    void Poll::FillEventArray(std::shared_ptr<ChannelList> channelList, int size)
    {
        for (int i = 0; i != size; i++){
            //mange the raw pointer manually
            std::shared_ptr<Channel::Channel> ch = static_cast<Channel::Channel*>(eventArray[i].data.ptr)->GetObject();
            ch->SetEvent(eventArray[i].events);
            channelList->push_back(ch);
        }
        return;
    }
    int Poll::poll(std::shared_ptr<ChannelList> channelList)
    {
        int ret = 0;
        int allocSize = channelMap.size();
        assert(allocSize > 0);
        eventArray.reset(new PollEvent[allocSize]);
        ret = epoll_wait(epfd,
                        eventArray.get(),
                        channelMap.size(),
                        -1);
        FillEventArray(channelList, ret);
        return ret;
    }
}
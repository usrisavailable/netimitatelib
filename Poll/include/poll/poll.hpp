#pragma once

#include <channel/channel.hpp>
#include <vector>
#include <map>
#include <memory>
#include <unistd.h>
#include <cstdio>
#include <string>
#include <iostream>
#include <cassert>

namespace Channel{
    class Channel;
}

namespace Poll{

class Poll : public std::enable_shared_from_this<Poll>{
public:
    using ChannelMap = std::map<int, std::shared_ptr<Channel::Channel>>;
    using ChannelList = std::vector<std::shared_ptr<Channel::Channel>>;
    using PollEventArray = struct epoll_event[];
    using PollEvent = struct epoll_event;
    Poll();
    ~Poll();
    void RegistEvennt(std::shared_ptr<Channel::Channel>);
    void FillEventArray(std::shared_ptr<ChannelList> channelList, int size);
    int poll(std::shared_ptr<ChannelList> channelList);
    std::shared_ptr<Poll> GetRawObject(){
        return shared_from_this();
    }
    void RemoveChannel(const std::shared_ptr<Channel::Channel>&);
private:
    int epfd;
    std::unique_ptr<PollEventArray> eventArray;
    ChannelMap channelMap;  //poll管理全部有效的channel
};

}
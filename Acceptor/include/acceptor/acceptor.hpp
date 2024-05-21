#pragma once

#include <string>
#include <channel/channel.hpp>
#include <eventloop/eventloop.hpp>
#include <socket/socket.hpp>
#include <functional>

namespace Eventloop {
    class Eventloop;
}

namespace Channel {
    class Channel;
}

namespace Socket {
    class Socket;
}

namespace Acceptor
{
    //第二个参数用于获取对方IP地址
    //改为在构造函数中获取？
    using NewConnectionCallback = std::function<void(int, std::string&)>;

    class Acceptor {
    public:
        Acceptor(std::shared_ptr<Eventloop::Eventloop> loop, 
                    std::string host, std::string service);
        Acceptor(const Acceptor&) = delete;
        Acceptor(Acceptor&&) = delete;
        ~Acceptor();
        void SetNewConnectionCallback(const NewConnectionCallback& cb);
        bool IsListening();
        void Listen(bool);
    private:
        void handleRead();
    private:
        std::shared_ptr<Eventloop::Eventloop> loop;
        Socket::Socket acceptSocket;
        std::shared_ptr<Channel::Channel> acceptChannel;
        NewConnectionCallback acceptCallback;
        bool isListening;
    };
} // namespace Acceptor

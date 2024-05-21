#pragma once

#include <functional>
#include <string>
#include <memory>
#include <string>

#include <eventloop/eventloop.hpp>
#include <channel/channel.hpp>
#include <socket/socket.hpp>
#include <buffer/buffer.hpp>

#define LOG(fmt, ...) \
    fprintf(stderr, "line%d: " fmt "\n", __LINE__, __VA_ARGS__)

namespace Eventloop {
    class Eventloop;
}

namespace Channel {
    class Channel;
}

namespace Socket {
    class Socket;
}

namespace Buffer {
    class Buffer;
}

namespace Tcpconn
{
    class Tcpconn;
    using MessageCallBack = std::function<void(std::shared_ptr<Tcpconn>,std::shared_ptr<Buffer::Buffer>)>;
    
    using ConnectionCallBack = std::function<void(std::string, int)>;
    using CloseCallBack = std::function<void(const std::shared_ptr<Tcpconn>&)>;
    enum kState{kConnected, kClosed};

    class Tcpconn : public std::enable_shared_from_this<Tcpconn> {
    public:
        Tcpconn(std::shared_ptr<Eventloop::Eventloop> loop, int fd, std::string address);
        Tcpconn(const Tcpconn&) = delete;
        Tcpconn(Tcpconn&&) = delete;
        Tcpconn& operator=(const Tcpconn&) = delete;
        Tcpconn& operator=(Tcpconn&&) = delete;
        ~Tcpconn();

        //回调函数的初始化
        void SetConnectionCallBack(const ConnectionCallBack&);
        void SetMessageCallBack(const MessageCallBack&);
        void SetCloseCallBack(const CloseCallBack&);

        void ConnectEstablished();
        void ConnectDestroyed();

        std::shared_ptr<Tcpconn> GetObject() {
            return shared_from_this();
        }
        std::shared_ptr<Eventloop::Eventloop> GetIOloop() {
            return this->loop->GetObject();
        }

        void Send(const std::string&);
        void Shutdown();

    private:
        //处理具体的连接事务
        void handleRead();
        void handleWrite();
        void handleClose();

        void sendInLoop(const std::string&);
        void shutDownInloop();

    private:
        std::shared_ptr<Eventloop::Eventloop> loop;
        std::shared_ptr<Channel::Channel> socketChannel;
        std::shared_ptr<Socket::Socket> connectSocket;
        std::string localAddress;
        std::string peerAddress;
        std::shared_ptr<Buffer::Buffer> inBuffer;
        std::shared_ptr<Buffer::Buffer> outBuffer;
        MessageCallBack msgCallBack;
        ConnectionCallBack connectionCallBack;
        CloseCallBack closeCallback;
        kState state;
    };
} // namespace Tcpconn

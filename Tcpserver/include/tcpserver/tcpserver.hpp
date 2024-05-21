#pragma once

#include <string>
#include <memory>
#include <unordered_set>
#include <functional>

#include <eventloop/eventloop.hpp>
#include <tcpconn/tcpconn.hpp>
#include <acceptor/acceptor.hpp>
#include <threadpoll/threadpoll.hpp>

namespace Eventloop {
    class Eventloop;
}

namespace Tcpconn {
    class Tcpconn;
}

namespace Acceptor {
    class Acceptor;
}

namespace Tcpserver
{
    class Tcpserver {
    public:
        Tcpserver(std::shared_ptr<Eventloop::Eventloop> loop, std::string host, std::string service);
        Tcpserver(const Tcpserver&) = delete;
        Tcpserver& operator=(const Tcpserver&) = delete;
        Tcpserver(Tcpserver&&) = delete;
        Tcpserver& operator=(Tcpserver&&) = delete;
        ~Tcpserver();
        /**
         * 初始化工作，启动监听
        */
        void Start(bool);
        void RemoveTcpConn(const std::shared_ptr<Tcpconn::Tcpconn>&);
        void RemoveTcpConnInloop(const std::shared_ptr<Tcpconn::Tcpconn>&);
        void SetConnectionCallBack(const Tcpconn::ConnectionCallBack&);
        void SetMessageCallBack(const Tcpconn::MessageCallBack&);
        void SetCloseCallBack(const Tcpconn::CloseCallBack&);
        //初始化线程池
        void SetThreadNums(int nums) {
            this->threadPoll->SetThreadNums(nums);
        }
    private:
        /**
         * 提供给Acceptor调用
         * @para fd 建立连接的socket； address 对方地址
        */
        void tcpserverConnection(int fd, std::string& address);
    private:
        std::shared_ptr<Eventloop::Eventloop> loop;
        std::unordered_set<std::shared_ptr<Tcpconn::Tcpconn>> tcpconnList;
        Tcpconn::ConnectionCallBack tcpConnectionCallBack;
        Tcpconn::MessageCallBack tcpMessageCallBack;
        Tcpconn::CloseCallBack tcpCloseCallBack;
        std::unique_ptr<Acceptor::Acceptor> acceptor;
        std::unique_ptr<Threadpoll::Threadpoll> threadPoll;

    };
} // namespace Tcpserver

#include "tcpserver/tcpserver.hpp"

namespace Tcpserver {
    Tcpserver::Tcpserver(std::shared_ptr<Eventloop::Eventloop> loop, std::string host, std::string service) 
    :loop(loop),
    acceptor(std::make_unique<Acceptor::Acceptor>(loop, host, service)),
    threadPoll(std::make_unique<Threadpoll::Threadpoll>(loop))
    {
        //成员函数列表初始化中，set对象会执行默认初始化
        //由于这个原因，其内部存储的对象不能是原始对象
        //因为Tcpconn的默认构造函数是delete状态，初始化会报错
    }

    Tcpserver::~Tcpserver() {
        this->tcpconnList.clear();
    }

    void Tcpserver::tcpserverConnection(int fd, std::string& address) {
        //tcpconn的初始化工作
        //Socket和Acceptor类都没有容错处理，因此这里必须完成这部分工作
        if (fd < 0)
            return;
        std::shared_ptr<Tcpconn::Tcpconn> conn = std::make_shared<Tcpconn::Tcpconn>(this->threadPoll->GetNextLoop(), fd, address);
        this->tcpconnList.insert(conn);
        conn->SetConnectionCallBack(this->tcpConnectionCallBack);
        conn->SetMessageCallBack(this->tcpMessageCallBack);
        conn->SetCloseCallBack(this->tcpCloseCallBack);
        conn->ConnectEstablished();
        return;
    }
    void Tcpserver::Start(bool openReuseAddr) {
        //线程吃初始化
        this->threadPoll->Start();
        //如果没有用户自定的关闭动作，则执行默认的(或者不提供API给用户)
        if (!this->tcpCloseCallBack)
            this->tcpCloseCallBack = std::bind(&Tcpserver::RemoveTcpConnInloop, this
                                                , std::placeholders::_1);
        auto acceptCallback = std::bind(&Tcpserver::tcpserverConnection, this, 
                                        std::placeholders::_1, 
                                        std::placeholders::_2);
        this->acceptor->SetNewConnectionCallback(acceptCallback);
        if(!this->acceptor->IsListening())
            this->acceptor->Listen(openReuseAddr);
        return;
    }

    void Tcpserver::SetConnectionCallBack(const Tcpconn::ConnectionCallBack& cb) {
        this->tcpConnectionCallBack = cb;
    }
    void Tcpserver::SetMessageCallBack(const Tcpconn::MessageCallBack& cb) {
        this->tcpMessageCallBack = cb;
    }
    //非必须
    void Tcpserver::SetCloseCallBack(const Tcpconn::CloseCallBack& cb) {
            this->tcpCloseCallBack = cb;
    }
    void Tcpserver::RemoveTcpConnInloop(const std::shared_ptr<Tcpconn::Tcpconn>& conn) {
        //在conn所在的loop中结束
        conn->GetIOloop()->queInLoop(std::bind(&Tcpserver::RemoveTcpConn, this, conn));
    }
    void Tcpserver::RemoveTcpConn(const std::shared_ptr<Tcpconn::Tcpconn>& conn) {
        conn->GetIOloop()->assertInLoop();
        auto iter = this->tcpconnList.find(conn);
        if (iter == this->tcpconnList.end())
            return;
        this->tcpconnList.erase(iter);
        return;
    }

}
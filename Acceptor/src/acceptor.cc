#include "acceptor/acceptor.hpp"

namespace Acceptor {
    Acceptor::Acceptor(std::shared_ptr<Eventloop::Eventloop> loop, 
                    std::string host, std::string service)
    : loop(loop), acceptSocket(host, service), isListening(false)
    {
        //the rest data member left other place
    }
    Acceptor::~Acceptor() {
        
    }

    bool Acceptor::IsListening() {
        return this->isListening;
    }

    void Acceptor::SetNewConnectionCallback(const NewConnectionCallback& cb) {
        this->acceptCallback = cb;
    }

    void Acceptor::Listen(bool openReuseAddr) {
        this->acceptSocket.Bind(openReuseAddr);
        this->acceptSocket.Listen();
        this->isListening = true;
        
        this->acceptChannel = std::make_shared<Channel::Channel>(this->loop->GetObject(), this->acceptSocket.GetSocketFd());
        this->acceptChannel->SetReadingCallBack(std::bind(&Acceptor::handleRead, this));
        this->acceptChannel->EnableReading();
        return;

    }
    void Acceptor::handleRead() {
        std::string peerAddress;
        int cfd = this->acceptSocket.Accept(peerAddress);
        this->acceptCallback(cfd, peerAddress);
        return;
    }
}
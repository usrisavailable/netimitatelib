#include "tcpconn/tcpconn.hpp"

namespace Tcpconn {
    Tcpconn::Tcpconn(std::shared_ptr<Eventloop::Eventloop> loop, int fd, std::string address) :
    loop(loop), 
    socketChannel(std::make_shared<Channel::Channel>(loop, fd)), 
    connectSocket(std::make_shared<Socket::Socket>()), 
    peerAddress(address),
    inBuffer(std::make_shared<Buffer::Buffer>()),
    outBuffer(std::make_shared<Buffer::Buffer>())
    {
        this->connectSocket->SetSocketFd(fd);
        this->state = kState::kConnected;
    }

    Tcpconn::~Tcpconn() {}

    void Tcpconn::ConnectEstablished() {
        this->connectionCallBack(this->peerAddress, this->state);
        this->socketChannel->SetReadingCallBack(std::bind(&Tcpconn::handleRead, this));
        this->socketChannel->SetCloseCallBack(std::bind(&Tcpconn::handleClose, this));
        this->socketChannel->SetWritingCallBack(std::bind(&Tcpconn::handleWrite, this));
        this->socketChannel->EnableReading();
        return;
    }

    void Tcpconn::handleRead() {
        //原封不懂的把收到的数据传回去
        //后面改成buffer形式的缓冲区，完整的接收数据
        int ret = this->inBuffer->ReadToBuf(this->socketChannel->GetFd());
        if (ret == -1) {
            handleClose();
            return;
        }

        if (ret == 0) {
            //end of file or other case
            ConnectDestroyed();
        }
        //buf[ret] = '\0';
        this->msgCallBack(shared_from_this(), this->inBuffer->GetObject());
        return;
    }

    void Tcpconn::handleWrite() {
        //将缓冲区中的数据输出给对端
        if (this->socketChannel->IsWriting()) {
            int nwrite = ::write(this->socketChannel->GetFd(),
                                this->outBuffer->peek(),
                                this->outBuffer->ReadableBytes());
            if (nwrite > 0) {
                //只调用一次，需要做后续处理
                this->outBuffer->RetrieveAsString(nwrite);
                if (this->outBuffer->ReadableBytes() == 0) {
                    this->socketChannel->DisableWriting();
                }
            } else {
                //write failed, maybe disconnected
            }
        } else {
            //no writing
        }
        return;
    }
    void Tcpconn::sendInLoop(const std::string& str) {
        int len = str.size();
        if (!this->socketChannel->IsWriting() && this->outBuffer->ReadableBytes() == 0) {
            int nwrite = ::write(this->socketChannel->GetFd(), 
                                str.c_str(), len);
            if (nwrite > 0 && nwrite < len) {
                const char* data = &*str.begin() + nwrite;
                std::string restStr(data, len - nwrite);
                //这个函数有问题，导致程序崩溃，各种段错误
                this->outBuffer->AppendInBuf(restStr);
                this->socketChannel->EnableWriting();
            } else {
                // write failed
            }
        } else {
            std::string restStr(str);
            this->outBuffer->AppendInBuf(restStr);
        }
        return;
    }
    void Tcpconn::Send(const std::string& str) {
        if (this->loop->IsInloop())
            this->sendInLoop(str);
        else
            this->loop->queInLoop(std::bind(&Tcpconn::sendInLoop, this, str));
        return;
    }

    void Tcpconn::Shutdown() {
        if (this->loop->IsInloop())
            this->shutDownInloop();
        else
            this->loop->queInLoop(std::bind(&Tcpconn::shutDownInloop, this));
        return;
    }

    void Tcpconn::shutDownInloop() {
        this->connectSocket->ShutdownWrite();
    }

    void Tcpconn::handleClose() {
        this->state = kState::kClosed;
        this->connectionCallBack(this->peerAddress, this->state);
        this->socketChannel->DisableAll();
        this->loop->RemoveChannel(this->socketChannel->GetObject());
        this->closeCallback(shared_from_this());
    }

    void Tcpconn::ConnectDestroyed() {
        //主动关闭连接
        //避免多次调用，导致double free
        if (this->state == kState::kClosed)
            return;
        this->loop->queInLoop(std::bind(&Tcpconn::handleClose, this));
        return;
    }

    void Tcpconn::SetConnectionCallBack(const ConnectionCallBack& cb) {
        this->connectionCallBack = cb;
    }
    void Tcpconn::SetMessageCallBack(const MessageCallBack& cb) {
        this->msgCallBack = cb;
    }
    //非必须
    void Tcpconn::SetCloseCallBack(const CloseCallBack& cb) {
            this->closeCallback = cb;
    }

}
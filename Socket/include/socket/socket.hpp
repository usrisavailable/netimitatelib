#pragma once

#include "socket/socket.hpp"
#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>
#include <string>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>

namespace Socket{
class Socket {
public:
    Socket();
    Socket(std::string service);
    Socket(std::string host, std::string service);
    //copy constructor and move constructor
    ~Socket();
public:
    /**
     * 三个建立连接的常规操作
     * 失败直接终止启动
    */
    void Bind(bool);
    void Listen();
    int Accept(std::string& peerAddress);
    /**
     * TCP连接的地址重用
    */
    void SetReuseAddr(bool);
    int GetSocketFd();
    /**
     * 特殊成员函数，保存未标记未listen状态的socket
    */
    void SetSocketFd(int fd) {
        this->socketfd = fd;
        return;
    }

    void ShutdownWrite() {
        ::shutdown(this->socketfd, SHUT_WR);
        return;
    }
private:
    int socketfd;
    bool resuseAddr;
    std::string host;
    std::string service;
};

}
#include "socket/socket.hpp"

namespace Socket
{
    Socket::Socket() : Socket("", "8080") {
        //委托构造函数
    }
    Socket::Socket(std::string service) :Socket("", service){
        //委托构造函数
    }
    Socket::Socket(std::string host, std::string service)
    : host(host), service(service)
    {
        //nothing
    }
    Socket::~Socket() {
        ::close(this->socketfd);
    }

    void Socket::Bind(bool openReueAddr) {
        struct addrinfo hint;
        struct addrinfo *result;
        hint.ai_addr = NULL;
        hint.ai_addrlen = 0;
        hint.ai_canonname = NULL;
        hint.ai_family = AF_INET;
        hint.ai_flags = 0;
        hint.ai_flags = AI_PASSIVE;
        hint.ai_next = NULL;
        hint.ai_protocol = 0;
        hint.ai_socktype = SOCK_STREAM;

        int ret = getaddrinfo(this->host == "" ? NULL : this->host.c_str(), 
                            this->service.c_str(),
                            &hint, &result);
        if (0 != ret) {
            perror("Socket Bind failed!");
            exit(1);
        }
        for (struct addrinfo *rp = result; rp != NULL; rp = rp->ai_next)
        {
            int sfd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
            if (sfd == -1)
                continue;
            this->SetReuseAddr(true);
            int ret = bind(sfd, rp->ai_addr, rp->ai_addrlen);
            if (-1 == ret) {
                ::close(sfd);
                perror("bind failed");
                exit(1);
            } 
            //now bind success
            this->socketfd = sfd;
            return;
        }
        perror("no available socket!");
        exit(1);
    }

    void Socket::SetReuseAddr(bool on) {
        int optval = 1;
        if (on)
            ::setsockopt(this->socketfd, 
                        SOL_SOCKET, SO_REUSEADDR,
                        &optval, sizeof(optval));
        return;
    }

    void Socket::Listen() {
        //在sfd可被监听前进来的连接在listen执行成功后会被立即连接
        int ret = ::listen(this->socketfd, 5);
        if (-1 == ret) {
            ::close(this->socketfd);
            perror("listen failed");
            exit(1);
        }
        return;
    }

    int Socket::Accept(std::string& peerAddress) {
        //接受连接的策略有多种
        //长连接：每次处理一个连接
        //短连接：每次接受固定连接数目，或者知道没有连接接入为止
        //accept每次调用都会从连接队列中取出一个连接
        struct sockaddr_in client;
        socklen_t clientlen = sizeof (struct sockaddr);
        int connectfd = ::accept(this->socketfd, (struct sockaddr*)&client, &clientlen);
        if (connectfd < 0)
            perror("accept failed");
        else {
            // 获取对方的IP地址和端口号
            char clienAddress[INET_ADDRSTRLEN];
            ::inet_ntop(AF_INET, &(client.sin_addr), clienAddress, INET_ADDRSTRLEN);
            peerAddress = std::string(clienAddress);
        }
        return connectfd;
    }

    int Socket::GetSocketFd() {
        return this->socketfd;
    }
} // namespace Socket

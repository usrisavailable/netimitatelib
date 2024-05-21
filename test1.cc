#include <iostream>
#include <thread>
#include <cassert>
#include <functional>
#include <memory>
#include <unistd.h>
#include <string>
#include <cstdio>

#include "timestamp/timestamp.hpp"
#include "eventloop/eventloop.hpp"
#include "thread/thread.hpp"
#include "channel/channel.hpp"
#include <sys/timerfd.h>
#include "loginfo/loginfo.hpp"
#include "eventloopthread/eventloopthread.hpp"
#include "acceptor/acceptor.hpp"
#include "buffer/buffer.hpp"
#include "tcpserver/tcpserver.hpp"
#include "tcpconn/tcpconn.hpp"
#include "httplib.hpp"

#define LOG(fmt, ...) \
    fprintf(stderr, "line%d: " fmt "\n", __LINE__, __VA_ARGS__)

using namespace std;

mutex jpgmutex;

Thread::Thread::ThreadFunc t1f = [](){

    Loginfo::Loginfo::infoPut(string("the thread"),
                            this_thread::get_id(),
                            string("is loopping!"));

    std::shared_ptr<Eventloop::Eventloop> loop = std::make_shared<Eventloop::Eventloop>();
    int socketFd = timerfd_create(CLOCK_MONOTONIC, TFD_CLOEXEC |  TFD_NONBLOCK);
    std::shared_ptr<Channel::Channel> channel = std::make_shared<Channel::Channel>(loop, socketFd);
    /* auto timeCallback = [](int fd){
        uint64_t eight;
        ::read(fd, &eight, sizeof eight);
        Loginfo::Loginfo::infoPut(std::string("time expire, timer trigered!"));
    };
    channel->SetCallBack(std::bind(timeCallback, channel->GetFd()), 0); */
    channel->SetReadingCallBack([socketFd](){
        uint64_t eight;
        ::read(socketFd, &eight, sizeof eight);
        Loginfo::Loginfo::infoPut(std::string("time expire, timer trigered!"));
        });
     channel->EnableReading();
    struct itimerspec newValue;
    newValue.it_interval.tv_sec  = 2;
    newValue.it_interval.tv_nsec = 0;
    newValue.it_value.tv_sec = 2;
    newValue.it_interval.tv_nsec = 0;
    timerfd_settime(socketFd, 0, &newValue, NULL);

    int countDown = 5;
    while(countDown--)
        loop->loop();
    return;
};


Acceptor::NewConnectionCallback acceptCallback = [](int fd, string& peerAddress) {
    string greeting = "hello ";
    ::write(fd, greeting.c_str(), greeting.size());
    ::write(fd, peerAddress.c_str(), peerAddress.size());
    ::write(fd, "\n", 1);
    ::close(fd);
};

//在其他线程中，插入定时器任务
auto t2f = [](shared_ptr<Eventloop::Eventloop> loop) {
    loop->RunAfter([]{
            Loginfo::Loginfo::infoPut("run in thread",
                                    this_thread::get_id());
        }, 1000);
};

/* auto msgCallBack = [](shared_ptr<Tcpconn::Tcpconn> conn, shared_ptr<Buffer::Buffer> buf){
    string str = buf->RetrieveAsString();
    Loginfo::Loginfo::infoPut(str);
    //log("%s",str.c_str());
    conn->GetIOloop()->RunAfter([]{
            Loginfo::Loginfo::infoPut("run in thread",
                                    this_thread::get_id());
        }, 500);
    conn->Send(str);
}; */

auto generateInt = [](int left, int right) -> int {
    std::random_device rd;  // 随机设备作为种子
    std::mt19937 gen(rd()); // 使用 Mersenne Twister 引擎
    std::uniform_int_distribution<int> dist(left, right); // 定义范围为 [1, 5] 的均匀分布

    int randomNum = dist(gen); // 生成随机数
    return randomNum;
};
static vector<string> imageList;
auto GetImage = []() {
    vector<string> fileList{"../Resource/001.jpg","../Resource/002.jpg","../Resource/003.jpg","../Resource/004.jpg","../Resource/005.jpg"};
    for (auto iter = fileList.begin(); iter != fileList.end(); iter++) {
        int fd = open(iter->c_str(), O_RDWR);
        if (fd == -1)
            {
                perror("open jpg file failed!");
                exit(1);
            }
        struct stat statbuf;
        int ret = fstat(fd, &statbuf);
        if (ret == -1)
            {
                perror("fstat failed!");
                exit(1);
            }
        int length = statbuf.st_size;
        char* addr = static_cast< char *>(mmap(NULL, length, 
            PROT_READ, MAP_PRIVATE, fd, 0));
        if (addr == MAP_FAILED)
            {
                perror("mmap failed!");
                exit(1);
            }
        imageList.push_back(string(addr, length));
        ::close(fd);
        ::munmap(addr, length);
    }
    return;
};

/* auto msgCallBack = [](shared_ptr<Tcpconn::Tcpconn> conn, shared_ptr<Buffer::Buffer> buf) {
    int ret;
    time_t timeNow;
    time(&timeNow);
    std::string currentTime = ctime(&timeNow);
    //clear substring [pos, end)
    currentTime.erase(currentTime.find('\n'));
    currentTime.append(std::string("\r\n"));
    //construct http reponse head
    std::string ReponseHead = {
        "HTTP/1.1 200 OK\r\n"
        "Server: time web server\r\n"
        "Connection: close\r\n"
        "Content-type: text/plain\r\n"
    };
    //construct http reponse body
    std::string ReponseBody = {
            "the current time is:\r\n"
        };
    ReponseBody.append(currentTime);
    //only complete body, and acquire the real size
    ReponseHead.append(std::string("Content-Length: "));
    ReponseHead.append(std::to_string(ReponseBody.size()));
    ReponseHead.append(std::string("\r\n\r\n"));
    
    std::string sendBuf;
    sendBuf.append(ReponseHead);
    sendBuf.append(ReponseBody);

    conn->Send(sendBuf);

    std::cerr << "service is: TimeService" << std::endl;
    std::cerr << "the thread is: " << std::this_thread::get_id() << std::endl;
    return;
};  */

auto msgCallBack = [](shared_ptr<Tcpconn::Tcpconn> conn, shared_ptr<Buffer::Buffer> buf) {
    int ret;
        //construct the http reponse head
        std::string ReponseHead = {
            "HTTP/1.1 200 OK\r\n"
            "Server: time web server\r\n"
            "Connection: close\r\n"
            "Content-type: image/jpeg\r\n"
        };
        
        int rand = generateInt(1, 5);
        string ReponseBody(imageList[rand - 1]);
        //the length of ReponseBody must be correct, or failed
        ReponseHead.append(std::string("Content-Length: "));
        ReponseHead.append(std::to_string(ReponseBody.size()));
        ReponseHead.append(std::string("\r\n\r\n"));

        //ReponseHead and ReponseBody can be send separately
        std::string sendBuf;
        sendBuf.append(ReponseHead);
        sendBuf.append(ReponseBody);
        //std::cerr << "http content length is: " << ReponseBody.size() << std::endl;
        //one connection just send once
       
        conn->Send(sendBuf);
        //after provid static content, collect the resource
        //now the resource collect do not perform here
        //close(fd);
        std::cerr << "service is: ImageService" << std::endl;
        std::cerr << "the thread is: " << std::this_thread::get_id() << std::endl;
        return;
};

auto connCallBack = [](string address, int state) {
    if (state == 1)
        Loginfo::Loginfo::infoPut(address,
                            "is disconnected!");
    else
        Loginfo::Loginfo::infoPut(address,
                            "is connected!");
};

int main()
{
    Loginfo::Loginfo::infoPut(string("main porcess"),
                            this_thread::get_id(),
                            string("is starting!"));
    {
        Eventloop::IgonreSigPIpe ignoreSigpipe;
        GetImage();
        Eventloopthread::Eventloopthread eventloop;
        shared_ptr<Eventloop::Eventloop> loop = eventloop.StartLoop();
        Tcpserver::Tcpserver server(loop, "", "9901");
        server.SetConnectionCallBack(connCallBack);
        server.SetMessageCallBack(msgCallBack);
        //两个IO线程启动
        server.SetThreadNums(2);
        server.Start(true); 
        //std::thread t2(t2f, loop->GetObject());
        //t2.join();
        std::this_thread::sleep_for(std::chrono::milliseconds(500000));
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(5000));
    Loginfo::Loginfo::infoPut(string("main porcess"),
                            this_thread::get_id(),
                            string("is ending!"));
    return 0;
}
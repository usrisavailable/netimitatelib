#include "threadpoll/threadpoll.hpp"

namespace Threadpoll {
    Threadpoll::Threadpoll(std::shared_ptr<Eventloop::Eventloop> baseLoop)
    :baseLoop(baseLoop), next(0), numOfThread(0) {
        //nothing
        //baseLoop is for Tcpserver
    }
    Threadpoll::~Threadpoll() {
        //nothing to do
        //智能指针和栈上对象都会自动释放
    }

    void Threadpoll::Start() {
        std::shared_ptr<Eventloopthread::Eventloopthread> t1;
        for (int i = 0; i < this->numOfThread; i++) {
            t1 = std::make_shared<Eventloopthread::Eventloopthread>();
            this->threadPoll.push_back(t1);
            this->loopList.push_back(t1->StartLoop());
        }
        return;
    }
    std::shared_ptr<Eventloop::Eventloop>
    Threadpoll::GetNextLoop() {
        //轮询调度
        std::shared_ptr<Eventloop::Eventloop> loop;
        if (!this->loopList.empty()) {
            loop = this->loopList[next % this->loopList.size()]->GetObject();
            next++;
        } else {
            loop = baseLoop->GetObject();
        }
        return loop;
    }

}
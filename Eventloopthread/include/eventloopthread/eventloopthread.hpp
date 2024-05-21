#pragma once

#include <eventloop/eventloop.hpp>
#include <thread/thread.hpp>
#include <condition_variable>
#include <mutex>

namespace Eventloopthread {

class Eventloopthread {
public:
    Eventloopthread();
    ~Eventloopthread();
    std::shared_ptr<Eventloop::Eventloop>& StartLoop();
private:
    void threadFunc();
private:
    Thread::Thread threadLoop;
    std::mutex conditonLock;
    std::condition_variable conditionV;
    std::shared_ptr<Eventloop::Eventloop> loop;
    bool exiting;   //作用是什么？
    std::atomic<bool> loopIsInit;   //好像没啥必要
};

}
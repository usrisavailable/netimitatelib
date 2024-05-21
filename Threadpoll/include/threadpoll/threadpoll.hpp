#pragma once

#include<memory>
#include<vector>
#include "eventloop/eventloop.hpp"
#include "eventloopthread/eventloopthread.hpp"

namespace Eventloop {
    class Eventloop;
}

namespace Eventloopthread {
    class Eventloopthread;
}

namespace Threadpoll {
    class Threadpoll {
    public:
        Threadpoll(std::shared_ptr<Eventloop::Eventloop> baseLoop);
        Threadpoll(const Threadpoll&) = delete;
        Threadpoll(Threadpoll&&) = delete;
        Threadpoll& operator=(const Threadpoll&) = delete;
        Threadpoll& operator=(Threadpoll&&) = delete;
        ~Threadpoll();
        void SetThreadNums(int nums) {
            this->numOfThread = nums;
        }
        void Start();
        std::shared_ptr<Eventloop::Eventloop> GetNextLoop();
    private:
        std::vector<std::shared_ptr<Eventloop::Eventloop>> loopList;
        std::vector<std::shared_ptr<Eventloopthread::Eventloopthread>> threadPoll;
        int next;
        int numOfThread;
        std::shared_ptr<Eventloop::Eventloop> baseLoop;
    };
}
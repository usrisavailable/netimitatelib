#pragma once

#include <functional>
#include <memory>
#include <thread>

namespace Thread{

class Thread{
public:
    using ThreadFunc = std::function<void()>;
    explicit Thread(const ThreadFunc& tf);
    ~Thread();
    void start();
private:
    void join();
private:
    ThreadFunc tf;
    std::unique_ptr<std::thread> t;
};

}
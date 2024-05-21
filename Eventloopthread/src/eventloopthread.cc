#include "eventloopthread/eventloopthread.hpp"

namespace Eventloopthread {
    Eventloopthread::Eventloopthread():
    threadLoop(std::bind(&Eventloopthread::threadFunc, this)),
    exiting(false),
    loopIsInit(false)
    {
        //线程是否要在这里启动？
    }

    Eventloopthread::~Eventloopthread()
    {
        this->loop->quit();
        this->exiting = true;
    }

    void Eventloopthread::threadFunc() {
        //这里不能加锁，会导致unique_lock加锁失败。
        //The effects of notify_one()/notify_all() and each of the three atomic parts of wait()/wait_for()/wait_until() (unlock+wait, wakeup, and lock) take place in a single total order that can be viewed as modification order of an atomic variable: the order is specific to this individual condition variable. This makes it impossible for notify_one() to, for example, be delayed and unblock a thread that started waiting just after the call to notify_one() was made.
        //意思是wait一定先于notify执行，编译器保证执行顺序
        //he notifying thread does not need to hold the lock on the same mutex as the one held by the waiting thread(s); in fact doing so is a pessimization, since the notified thread would immediately block again, waiting for the notifying thread to release the lock. However, some implementations (in particular many implementations of pthreads) recognize this situation and avoid this "hurry up and wait" scenario by transferring the waiting thread from the condition variable's queue directly to the queue of the mutex within the notify call, without waking it up.
        //被通知的线程会立刻加锁，因此没必要在通知线程加锁
        //std::lock_guard<std::mutex> lock(this->conditonLock);
        this->loop = std::make_shared<Eventloop::Eventloop>();
        this->loop->Init();
        this->loopIsInit.store(true);
        this->conditionV.notify_one();
        this->loop->loop();
        return;
    }

    std::shared_ptr<Eventloop::Eventloop>&
    Eventloopthread::StartLoop() {
        this->threadLoop.start();
        std::unique_lock<std::mutex> lock(this->conditonLock);
        //使用reload版本的wait, 避免虚假唤醒的影响
        this->conditionV.wait(lock, [this]{
            return this->loopIsInit.load();
        });
        lock.unlock();
        return this->loop;
    }

}
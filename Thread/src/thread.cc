#include "thread/thread.hpp"

namespace Thread{

Thread::Thread(const ThreadFunc& tf)
    :tf(tf), t(nullptr)
{
    //do nothing
}

Thread::~Thread()
{
    //等待所有子线程退出，
    if (t->joinable())
        t->join();
}

void Thread::start(){
    //delay initialization here
    this->t = std::make_unique<std::thread>(std::thread(tf));
    //this->t.reset(new std::thread(tf));
    return;
}

void Thread::join()
{
    if (t->joinable())
        t->join();
    return;
}

}
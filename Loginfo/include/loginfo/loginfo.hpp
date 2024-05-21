#pragma once

#include <mutex>
#include <iostream>

#include <timestamp/timestamp.hpp>



//模板函数是定义和实现需要放在同一个文件中，不然实例化的时候会undefined reference

namespace Loginfo{

class Loginfo{
public:
    //delete ctor and dtor, include moving version

    template<typename... Args>
    static void infoPut(Args... args);

private:
template<typename T>
    static void printArgs(T lastArg);

    template<typename T, typename... Args>
    static void printArgs(T arg, Args... args);
private:
    static std::mutex logMutex;
};

template<typename T>
void Loginfo::printArgs(T lastArg)
{
    std::cout << lastArg << std::endl;
    return;
}

template<typename T, typename... Args>
void Loginfo::printArgs(T firstArg, Args... args)
{
    std::cout << firstArg << std::string(2, ' ');
    printArgs(args...);
    return;
}

template<typename... Args>
void Loginfo::infoPut(Args... args)
{
    std::lock_guard<std::mutex> lock(logMutex);
    //function parameter packet and template parameter  packet
    printArgs(TimeStamp::TimeStamp::getCurrentTime(), args...);
    return;
}

}
#include "buffer/buffer.hpp"

namespace Buffer {
    Buffer::Buffer()
    : data(kPrependSize + kInitSize),
      prependIndex(kPrependSize),
      readIndex(kPrependSize),
      readEndIndex(kPrependSize)
    {}
    Buffer::~Buffer() 
    {}

    //麻烦的地方一：何时扩容，扩容的触发条件
    int Buffer::ReadToBuf(int fd) {
        this->ensureBuf(this->writeAbleBytes());
        char *readStart = this->startRead();
        std::size_t writeable = this->writeAbleBytes();
        int nread = ::read(fd, readStart, writeable);
        //要不要处理EAGAIN和EWOULDBLOCK的非错误情况       待定
        if (nread < 0) {
          return nread;
        }
        this->readEndIndex += nread;
        return nread;
    }

    void Buffer::ensureBuf(long len) {
      if (len < ksentry)
        this->makeSpace(ksentry);
      else
        this->makeSpace(len);
      return;
    }
    
    void Buffer::makeSpace(long len) {
      long leftSpace = this->data.size() - this->readEndIndex;
      if (len <= leftSpace) {
        //make space in buffer
        //操作同一个容器
        std::copy(this->begin() + this->readIndex,
                  this->begin() + this->readEndIndex,
                  this->begin() + this->prependIndex);
        this->readEndIndex = this->prependIndex + (this->readEndIndex - this->readIndex);
        this->readIndex = this->prependIndex;
       
      } else {
        //make space in heap
        long addNum = len < ksentry ? ksentry : len + ksentry;
        this->data.resize(this->data.size() + addNum);
      }
      return;
    }

    void Buffer::retrieveAll() {
      this->readIndex = this->prependIndex;
      this->readEndIndex = this->prependIndex;
    }
    void Buffer::retrievePart(int n) {
      this->readIndex += n;
    }

    std::string Buffer::RetrieveAsString() {
      std::string str = std::string(this->startRead(), this->ReadableBytes());
      this->retrieveAll();
      return str;
    }
    std::string Buffer::RetrieveAsString(int n) {
      int nread = n <= this->ReadableBytes() ? n : this->ReadableBytes();
      std::string str = std::string(this->startRead(), nread);
      this->retrievePart(nread);
      return str;
    }

    int Buffer::AppendInBuf(std::string& str) {
      //写入的成员方法就这一个
      this->ensureBuf(str.size());
      //std::vector<char> destVec;
      //操作不同的容器
      //std::copy(str.begin(), str.end(),
       //         std::back_inserter(destVec));
  
      const char* outputData = str.c_str();
      int len = str.size();
      //mkspace函数判断条件有误，每次值增加1000字节，导致每次容量都不足
      //容器容量远小于写入容量
      //copy时候崩溃了
      //LOG("%d %d", this->data.size(), len);
      std::copy(outputData, outputData + len, this->begin() + this->readEndIndex);
      return len;
    }
}
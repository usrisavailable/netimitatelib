#pragma once

#include <vector>
#include <functional>
#include <memory>
#include <string>
#include <unistd.h>
#include <cstdio>

#define LOG(fmt, ...) \
    fprintf(stderr, "line%d: " fmt "\n", __LINE__, __VA_ARGS__)

namespace Buffer {
    /**
     * +------------------------------------------------------+
     * |   prependable bytes    |       readable buytes       |
     * |                        |             content         |
     * +------------------------------------------------------+
     * 0          ======>   readIndex       ======>          size()
    */
    //buffer的设计比较简单,整个区域从头部预留8个字节，其他都是payload
    //Buffer类的核心就是自己管理缓冲区的下标，如上，就三个
    //内部使用标准库的vector，使用连续的存储空间
    static const int kPrependSize = 8;
    static const int kInitSize = 8196;
    static const int ksentry = 1024;

    class Buffer : public std::enable_shared_from_this<Buffer> {
    public:
        Buffer();
        Buffer(const Buffer&) = default;
        Buffer(Buffer&&) = default;
        Buffer& operator=(const Buffer&) = default;
        Buffer& operator=(Buffer&&) = default;
        ~Buffer();
        /**
         * 从fd中读取数据
         * 空间足够则存入缓冲区，空间不足则扩容再存入读取的字节
         * @para 
         * @ret the result of system call "read"
        */
        int ReadToBuf(int fd);
        /**
         * 操作缓冲区的三个API
        */
        std::string RetrieveAsString();
        std::string RetrieveAsString(int);
        /**
         * 当前缓冲区容量
        */
       int ReadableBytes() {
        return this->readEndIndex - this->readIndex;
       }

        int AppendInBuf(std::string&);
        std::shared_ptr<Buffer> GetObject() {
            return shared_from_this();
        }
        char* peek() {
            return this->startRead();
        }
    private:
        //封装一些基本操作
        char* startRead() {
            return this->begin() + this->readIndex;
        };
        char* begin() {
            return &*this->data.begin();
        }
        std::size_t writeAbleBytes() {
            return data.size() - readEndIndex;
        }
        //long类型，防止溢出
        void ensureBuf(long);
        void makeSpace(long);
        //对应上面开放的两个API
        void retrieveAll();
        void retrievePart(int);
    private: 
        int prependIndex;
        long readIndex;
        long readEndIndex;
        std::vector<char> data;
    };
}
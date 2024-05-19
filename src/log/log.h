/*
 * @author: Zimo Li
 * @date: 2024-5-18
*/

#ifndef LOG_H
#define LOG_H

#include "../buffer/buffer.h"
#include <memory>
#include "blockdeque.h"
#include <thread>
#include <mutex>

class Log {
private:
    static Log instance;

    static const int LOG_PATH_LEN = 256;
    static const int LOG_NAME_LEN = 256;
    static const int MAX_LINES = 50000;

    const char* path;
    const char* suffix;

    int MAX_LINES_;

    int lineCount;
    int today;

    bool isOpen;

    Buffer buffer;
    int level;
    bool isAsync;

    FILE* fp;
    std::unique_ptr<BlockDeque<std::string>> deque_;
    std::unique_ptr<std::thread> writeThread;
    std::mutex mtx;

    Log();

public:
    Log(const Log&) = delete;
    Log& operator=(const Log&) = delete;

    void init(int level = 1, const char* path, const char* suffix, int maxDequeSize);
};

Log Log::instance;

#endif // LOG_H

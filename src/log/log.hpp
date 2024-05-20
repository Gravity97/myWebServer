/*
 * @author: Zimo Li
 * @date: 2024-5-18
*/

#ifndef LOG_HPP
#define LOG_HPP

#include "../buffer/buffer.hpp"
#include <memory>
#include "blockdeque.hpp"
#include <thread>
#include <mutex>

class Log {
private:
    static Log instance;

    static const int LOG_PATH_LEN = 256;
    static const int LOG_NAME_LEN = 64;
    static const int MAX_LINE_LEN = 128;
    static const int MAX_LINES = 50000;

    const char* path;
    const char* suffix;

    bool isOpen;
    int lineCount;
    int today;

    Buffer buffer;
    int level;
    bool isAsync; // if log is async, we will have a new thread to process log

    FILE* fp;
    std::unique_ptr<BlockDeque<std::string>> deque_;
    std::unique_ptr<std::thread> writeThread;
    mutable std::mutex mtx;

    Log() : lineCount(0), today(0), isAsync(false), 
            fp(nullptr), deque_(nullptr), writeThread(nullptr) {};
    ~Log();

    void AppendLogTitle(int level);
    void AsyncWrite();

public:
    Log(const Log&) = delete;
    Log& operator=(const Log&) = delete;

    static Log& Instance();
    void init(int level = 1, const char* path = "./log",
             const char* suffix = ".log", int maxDequeSize = 1024);
    bool IsOpen() const;

    int GetLevel() const;
    void SetLevel(int level);

    void flush();

    void write(int level, const char* format, ...);
};

#define LOG_BASE(level, format, ...) \
    do {\
        Log& instance = Log::Instance(); \
        if(instance.IsOpen() && instance.GetLevel() <= level) {\
            instance.write(level, format, ##__VA_ARGS__); \
            instance.flush(); \
        }\
    } while(0);

#define LOG_DEBUG(format, ...) do{LOG_BASE(0, format, ##__VA_ARGS__)} while(0);
#define LOG_INFO(format, ...) do{LOG_BASE(1, format, ##__VA_ARGS__)} while(0);
#define LOG_WARN(format, ...) do{LOG_BASE(2, format, ##__VA_ARGS__)} while(0);
#define LOG_ERROR(format, ...) do{LOG_BASE(3, format, ##__VA_ARGS__)} while(0);

#endif // LOG_HPP

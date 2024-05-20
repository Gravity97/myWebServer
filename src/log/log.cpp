/*
 * @author: Zimo Li
 * @date: 2024-5-19
*/

#include "log.hpp"
#include <sys/stat.h>
#include <assert.h>
#include <sys/time.h>
#include <cstdarg>

Log Log::instance;

Log::~Log()
{
    if(writeThread && writeThread->joinable()) {
        while(!deque_->empty()) {
            deque_->flush();
        }
        deque_->close();
        writeThread->join();
    }
    
    if(fp) {
        std::lock_guard<std::mutex> locker(mtx);
        flush();
        fclose(fp);
    }
}

void Log::AppendLogTitle(int level)
{
    switch (level) {
    case 0:
        buffer.Append("[debug]: ", 9);
        break;
    case 1:
        buffer.Append("[info] : ", 9);
        break;
    case 2:
        buffer.Append("[warn] : ", 9);
        break;
    case 3:
        buffer.Append("[error]: ", 9);
        break;
    default:
        buffer.Append("[info] : ", 9);
        break;
    }
}

void Log::AsyncWrite()
{
    std::string str = "";
    while(deque_->pop(str)) {
        std::lock_guard<std::mutex> locker(mtx);
        fputs(str.c_str(), fp);
    }
}

Log& Log::Instance()
{
    return instance;
}

void Log::init(int level, const char* path, const char* suffix, int maxDequeSize)
{
    this->isOpen = true;
    this->level = level;
    this->lineCount = 0;

    if(maxDequeSize > 0) {
        isAsync = true;
        if(!deque_) {
            std::unique_ptr<BlockDeque<std::string>> newDeque(new BlockDeque<std::string>);
            deque_ = std::move(newDeque);

            std::unique_ptr<std::thread> newThread(new std::thread([]{
                Log::Instance().AsyncWrite();
            }));
            writeThread = std::move(newThread);
        }
    } else {
        isAsync = false;
    }

    time_t seconds = time(nullptr); // timer is the seconds from 1970-1-1 to now
    tm* ptr = localtime(&seconds); // switch seconds to tm struct ptr
    tm t = *ptr;

    this->path = path;
    this->suffix = suffix;
    this->today = t.tm_mday;

    char filename[LOG_PATH_LEN + 1] = {0};
    snprintf(filename, LOG_PATH_LEN, "%s/%04d_%02d_%02d%s",
            path, t.tm_year + 1900, t.tm_mon + 1, t.tm_mday, suffix);
    
    {
        std::lock_guard<std::mutex> locker(mtx);
        buffer.RetrieveAll();
        if(fp) {
            flush();
            fclose(fp);
        }

        fp = fopen(filename, "a");
        if(fp == nullptr) {
            mkdir(path, 0777);
            fp = fopen(filename, "a");
        }
        assert(fp != nullptr);
    }
}

bool Log::IsOpen() const
{
    std::lock_guard<std::mutex> locker(mtx);
    return isOpen;
}

int Log::GetLevel() const
{
    std::lock_guard<std::mutex> locker(mtx);
    return this->level;
}

void Log::SetLevel(int level)
{
    std::lock_guard<std::mutex> locker(mtx);
    this->level = level;
}

void Log::flush()
{
    if(isAsync) {
        deque_->flush();
    }
    fflush(fp);
}

void Log::write(int level, const char* format, ...)
{
    timeval mseconds = {0, 0};
    gettimeofday(&mseconds, nullptr);
    time_t seconds = mseconds.tv_sec;
    tm* ptr = localtime(&seconds);
    tm t = *ptr;

    if(today != t.tm_mday ||
     (lineCount && lineCount % MAX_LINES == 0)) {
        std::unique_lock<std::mutex> locker(mtx);
        locker.unlock();

        char newFile[LOG_PATH_LEN + 1] = {0};
        char name[LOG_NAME_LEN + 1] = {0};
        snprintf(name, LOG_NAME_LEN, "%04d_%02d_%02d",
                t.tm_year + 1900, t.tm_mon + 1, t.tm_mday);
        
        if(today != t.tm_mday) {
            snprintf(newFile, LOG_PATH_LEN, "%s/%s%s", path, name, suffix);
            today = t.tm_mday;
            lineCount = 0;
        } else {
            snprintf(newFile, LOG_PATH_LEN, "%s/%s-%d%s",
                    path, name, lineCount / MAX_LINES, suffix);
        }

        locker.lock();
        flush();
        fclose(fp);
        fp = fopen(newFile, "a");
        assert(fp != nullptr);
    }

    {
        // write log content
        std::lock_guard<std::mutex> locker(mtx);
        char time[MAX_LINE_LEN + 1] = {0};
        int n = snprintf(time, MAX_LINE_LEN, "%04d-%02d-%02d %02d:%02d:%02d.%06ld ",
                t.tm_year + 1900, t.tm_mon + 1, t.tm_mday,
                t.tm_hour, t.tm_min, t.tm_sec, mseconds.tv_usec);

        buffer.Append(time, n);
        AppendLogTitle(level);

        va_list valist;
        va_start(valist, format);
        char data[MAX_LINE_LEN - n + 1] = {0};
        int m = vsnprintf(data, MAX_LINE_LEN - n, format, valist);
        buffer.Append(data, m);
        va_end(valist);
        buffer.Append("\n\0", 2);

        if(isAsync && deque_ && !deque_->full()) {
            deque_->push_back(buffer.RetrieveAllToStr());
        } else {
            fputs(buffer.RetrieveAllToStr().c_str(), fp);
        }
    }
}

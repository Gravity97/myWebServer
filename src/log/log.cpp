/*
 * @author: Zimo Li
 * @date: 2024-5-19
*/

#include "log.h"
#include <sys/stat.h>

Log::Log()
{
    
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

            std::unique_ptr<std::thread> newThread(new std::thread);
            writeThread = std::move(newThread);
        }
    } else {
        isAsync = false;
    }

    // get current time
    time_t seconds = time(nullptr); // timer is the seconds from 1970-1-1 to now
    tm* ptr = localtime(&seconds); // switch seconds to tm struct ptr
    tm t = *ptr;

    this->path = path;
    this->suffix = suffix;
    this->today = t.tm_mday;

    char filename[LOG_NAME_LEN] = {0};
    snprintf(filename, LOG_NAME_LEN - 1, "%s/%04d_%02d_%02d%s",
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
    }
}

void Log::flush()
{
    if(isAsync) {
        deque_->flush();
    }
    fflush(fp);
}

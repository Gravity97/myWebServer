/*
 * @author: Zimo Li
 * @date: 2024-5-19
*/

#include "log.h"

Log::Log()
{
    
}

void Log::init(int level, const char* path, const char* suffix, int maxDequeSize)
{
    isOpen = true;
    this->level = level;

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


}
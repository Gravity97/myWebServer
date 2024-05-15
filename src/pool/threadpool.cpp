/*
 * @author: Zimo Li
 * @date: 2024-5-11
*/

#include <assert.h>
#include "threadpool.h"

ThreadPool::ThreadPool(size_t threadNum) : pool(std::make_shared<Pool>())
{
    assert(threadNum > 0);
    for(size_t i = 0; i < threadNum; i++){
        // start a new thread
        std::thread([pool = this->pool]() {
            std::unique_lock<std::mutex> locker(pool->mtx);
            while(true){
                if(!pool->tasks.empty()){
                    auto task = std::move(pool->tasks.front());
                    pool->tasks.pop();

                    // start excuting the task
                    locker.unlock();
                    task();
                    locker.lock();
                } 
                else if (pool->isClose) break;
                else pool->cond.wait(locker);
            }
        }).detach();
    }
}

ThreadPool::~ThreadPool()
{
    
}

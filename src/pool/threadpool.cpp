/*
 * @author: Zimo Li
 * @date: 2024-5-11
*/

#include <assert.h>
#include "threadpool.h"

ThreadPool::ThreadPool(size_t threadNum) : pool(std::make_shared<Pool>())
{
    assert(threadNum > 0);
    pool->isClose = false;
    for(size_t i = 0; i < threadNum; i++){
        // create a new thread
        std::thread([pool = this->pool]() {
            std::unique_lock<std::mutex> locker(pool->mtx); // if unique_lock can't get the lock, it will wait
            while(true){
                if(!pool->tasks.empty()){
                    auto task = std::move(pool->tasks.front());
                    pool->tasks.pop();
                    locker.unlock(); // other thread can access task queue
                    task(); // start excuting the task
                    locker.lock();
                } 
                else if (pool->isClose) break;
                else pool->cond.wait(locker); // wait() will unlock the mutex
            }
        }).detach();
    }
}

ThreadPool::~ThreadPool()
{
    if(static_cast<bool>(pool)){
        {
            std::lock_guard<std::mutex> lock_guard(pool->mtx);
            pool->isClose = true;
        }
        pool->cond.notify_all();
    }
}

template<class F>
void ThreadPool::addTask(F&& task)
{
    {
        std::lock_guard<std::mutex> locker(pool->mtx);
        pool->tasks.emplace(std::forward<F>(task));
        // pool->tasks.emplace(std::forward(task));
    }
    pool->cond.notify_all();
}

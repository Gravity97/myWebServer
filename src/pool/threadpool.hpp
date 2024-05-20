/*
 * @author: Zimo Li
 * @date: 2024-5-11
*/

#ifndef THREADPOOL_HPP
#define THREADPOOL_HPP

#include <queue>
#include <thread>
#include <functional>
#include <mutex>
#include <condition_variable>

class ThreadPool {
private:
    struct Pool{
        bool isClose; // thread pool is closed or not
        std::mutex mtx; // mutex for the thread pool
        std::condition_variable cond; // condition variable for the thread pool
        std::queue<std::function<void()>> tasks; // task queue
    };

    std::shared_ptr<Pool> pool;

public:
    ThreadPool() = default;

    explicit ThreadPool(size_t threadNum = 8);

    ThreadPool(ThreadPool&&) = default;

    ~ThreadPool();

    template<class F>
    void addTask(F&& task);
};

#endif //THREADPOOL_HPP

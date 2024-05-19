/*
 * @author: Zimo Li
 * @date: 2024-5-18
*/

#include "blockdeque.h"
#include <assert.h>

#define BLOCK_DEQUE_FUNC_DEFINE(FUNC, ...) \
    template <class T> \
    auto BlockDeque<T>::FUNC(__VA_ARGS__) -> decltype(deque_.FUNC(__VA_ARGS__)) const\
    { \
        std::lock_guard<std::mutex> locker(mtx); \
        return deque_.FUNC(__VA_ARGS__); \
    }

template<class T>
BlockDeque<T>::BlockDeque(int maxCapacity) : capacity_(maxCapacity)
{
    assert(maxCapacity > 0);
    isClosed = false;
}

template <class T>
BlockDeque<T>::~BlockDeque()
{
    close();
}

#if USE_MACRO_FUNC
BLOCK_DEQUE_FUNC_DEFINE(size)

#else
template <class T>
std::size_t BlockDeque<T>::size() const
{
    std::lock_guard<std::mutex> locker(mtx);
    return deque_.size();
}
#endif

template <class T>
std::size_t BlockDeque<T>::capacity() const
{
    std::lock_guard<std::mutex> locker(mtx);
    return capacity_;
}

template <class T>
bool BlockDeque<T>::full() const
{
    std::lock_guard<std::mutex> locker(mtx);
    return deque_.size >= capacity_;
}

#if USE_MACRO_FUNC
BLOCK_DEQUE_FUNC_DEFINE(empty)
BLOCK_DEQUE_FUNC_DEFINE(clear)
BLOCK_DEQUE_FUNC_DEFINE(front)
BLOCK_DEQUE_FUNC_DEFINE(back)

#else
template <class T>
bool BlockDeque<T>::empty() const
{
    std::lock_guard<std::mutex> locker(mtx);
    return deque_.empty();
}

template <class T>
void BlockDeque<T>::clear() {
    std::lock_guard<std::mutex> locker(mtx);
    return deque_.clear();
}

template <class T>
T BlockDeque<T>::front() const
{
    std::lock_guard<std::mutex> locker(mtx);
    return deque_.front();
}

template <class T>
T BlockDeque<T>::back() const
{
    std::lock_guard<std::mutex> locker(mtx);
    return deque_.back();
}
#endif

template <class T>
void BlockDeque<T>::push_front(const T& item) {
    std::unique_lock<std::mutex> locker(mtx);
    while(deque_.size() >= capacity_) {
        condProducer.wait(locker);
        if(isClosed) return;
    }
    deque_.push_front(item);
    condConsumer.notify_one();
}

template <class T>
void BlockDeque<T>::push_back(const T& item)
{
    std::unique_lock<std::mutex> locker(mtx);
    while(deque_.size() >= capacity_) {
        condProducer.wait(locker);
        if(isClosed) return;
    }
    deque_.push_back(item);
    condConsumer.notify_one();
}

template <class T>
bool BlockDeque<T>::pop(T& item)
{
    std::unique_lock<std::mutex> locker(mtx);
    while(deque_.empty()) {
        condConsumer.wait(locker);
        if(isClosed) {
            return false;
        }
    }
    item = deque_.front();
    deque_.pop_front();
    condProducer.notify_one();
    return true;
}

template <class T>
bool BlockDeque<T>::pop(T& item, int timeout)
{
    std::unique_lock<std::mutex> locker(mtx);
    while(deque_.empty()) {
        if(condConsumer.wait_for(locker, 
            std::chrono::seconds(timeout)) == std::cv_status::timeout) {
                return false;
            }
        if(isClosed) {
            return false;
        }
    }
    item = deque_.front();
    deque_.pop_front();
    condProducer.notify_one();
    return true;
}

template <class T>
void BlockDeque<T>::flush()
{
    condConsumer.notify_one();
}

template <class T>
void BlockDeque<T>::close() {
    {
        std::lock_guard<std::mutex> locker(mtx);
        deque_.clear();
        isClosed = true;
    }
    condConsumer.notify_all();
    condProducer.notify_all();
}

#include <iostream>
#include <thread>
#include <vector>
#include "blockdeque.h"

int main() {
    BlockDeque<int> deque;
    std::vector<std::thread> producers;
    std::vector<std::thread> consumers;

    for (int i = 0; i < 10; ++i) {
        producers.push_back(std::thread([&deque, i] {
            deque.push_back(i);
        }));
    }

    for (int i = 0; i < 10; ++i) {
        consumers.push_back(std::thread([&deque] {
            int item;
            bool result = deque.pop(item);
            if (result) {
                std::cout << "Popped: " << item << std::endl;
            } else {
                std::cout << "Failed to pop" << std::endl;
            }
        }));
    }

    for (auto& t : producers) {
        t.join();
    }

    for (auto& t : consumers) {
        t.join();
    }

    if (deque.empty()) {
        std::cout << "Deque is empty" << std::endl;
    } else {
        std::cout << "Deque is not empty" << std::endl;
    }

    return 0;
}

/*
 * @author: Zimo Li
 * @date: 2024-5-18
*/

#ifndef BLOCKDEQUE_HPP
#define BLOCKDEQUE_HPP

#include <deque>
#include <mutex>
#include <condition_variable>
#include <cassert>

#define USE_MACRO_FUNC 1

#define BLOCK_DEQUE_FUNC_DECL(FUNC, CONST, ...) \
    auto FUNC(__VA_ARGS__) -> decltype(deque_.FUNC(__VA_ARGS__)) CONST;

// a deque used in multithread environment
template <class T>
class BlockDeque {
private:
    std::deque<T> deque_;
    std::size_t capacity_;

    mutable std::mutex mtx;
    bool isClosed;
    std::condition_variable condConsumer;
    std::condition_variable condProducer;

public:
    BlockDeque(int maxCapacity = 1024);
    ~BlockDeque();

#if USE_MACRO_FUNC
    BLOCK_DEQUE_FUNC_DECL(size, const)
#else
    std::size_t size() const;
#endif

    std::size_t capacity() const;

    bool full() const;

#if USE_MACRO_FUNC
    BLOCK_DEQUE_FUNC_DECL(empty, const)
    BLOCK_DEQUE_FUNC_DECL(clear, const)
    BLOCK_DEQUE_FUNC_DECL(front, const)
    BLOCK_DEQUE_FUNC_DECL(back, const)
#else
    bool empty() const;
    void clear();

    T front() const;
    T back() const;
#endif

    void push_front(const T& item);
    void push_back(const T& item);
    bool pop(T& item);
    bool pop(T& item, int timeout);

    void flush();
    void close();
};

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
    return deque_.size() >= capacity_;
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

#endif // BLOCKDEQUE_HPP
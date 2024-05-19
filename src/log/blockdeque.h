/*
 * @author: Zimo Li
 * @date: 2024-5-18
*/

#ifndef BLOCKDEQUE_H
#define BLOCKDEQUE_H

#include <deque>
#include <mutex>
#include <condition_variable>

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

#endif // BLOCKDEQUE_H

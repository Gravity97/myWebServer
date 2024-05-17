/*
 * @author: Zimo Li
 * @date: 2024-5-16
*/

#ifndef BUFFER_H
#define BUFFER_H

#include <vector>
#include <atomic>

class Buffer {
    private:
        std::vector<char> buffer;
        std::atomic<std::size_t> readPos; // point to read position
        std::atomic<std::size_t> writePos; // point to write position

    public:
        Buffer(int bufferSize = 1024) : buffer(bufferSize), readPos(0), writePos(0) {};
        ~Buffer() = default;
};


#endif //BUFFER_H
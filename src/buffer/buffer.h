/*
 * @author: Zimo Li
 * @date: 2024-5-16
*/

#ifndef BUFFER_H
#define BUFFER_H

#include <vector>
#include <atomic>
#include <string>

class Buffer {
    private:
        std::vector<char> buffer;
        std::atomic<std::size_t> readPos; // point to read position(all read before)
        std::atomic<std::size_t> writePos; // point to write position(all written before)

        void MakeSpace(std::size_t len);

    public:
        Buffer(int bufferSize = 1024) : buffer(bufferSize), readPos(0), writePos(0) {};
        ~Buffer() = default;

        std::size_t WritableBytes() const;
        std::size_t ReadableBytes() const;
        std::size_t PreinsertableBytes() const;

        const char* ReadPosition() const;
        const char* WritePosition() const;
        void EnsureWritable(std::size_t len);
        void HasRead(std::size_t len); // note the bytes that has read
        void HasWritten(std::size_t len); // note the bytes that has written

        void Retrieve(std::size_t len);
        void RetrieveUntil(const char* end);
        void RetrieveAll();
        std::string RetrieveAllToStr();

        void Append(const char* str, size_t len);
        void Append(const std::string& str);
        void Append(const void* data, size_t len);
        void Append(const Buffer& buff);

        ssize_t ReadFromFd(int fd, int* errno_);
        ssize_t WriteIntoFd(int fd, int* errno_);
};

#endif //BUFFER_H
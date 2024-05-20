/*
 * @author: Zimo Li
 * @date: 2024-5-16
*/

#ifndef BUFFER_HPP
#define BUFFER_HPP

#include <vector>
#include <atomic>
#include <string>

class Buffer {
private:
    std::vector<char> buffer;
    std::atomic<std::size_t> readPos; // point to read position(all read before)
    std::atomic<std::size_t> writePos; // point to write position(all written before)

    const char* ReadPosition() const; // position for read pointer
    char* WritePosition(); // position for write pointer
    void EnsureWritable(std::size_t len); // make sure the write space
    void HasRead(std::size_t len); // note the bytes that has read
    void HasWritten(std::size_t len); // note the bytes that has written

    void MakeSpace(std::size_t len);

public:
    Buffer(int bufferSize = 1024) : buffer(bufferSize), readPos(0), writePos(0) {};
    ~Buffer() = default;

    std::size_t WritableBytes() const;
    std::size_t ReadableBytes() const;
    std::size_t PreinsertableBytes() const;

    void Retrieve(std::size_t len); // get readable data
    void RetrieveUntil(const char* end);
    void RetrieveAll();
    std::string RetrieveAllToStr();

    void Append(const char* str, size_t len); // write new data
    void Append(const std::string& str);
    void Append(const void* data, size_t len);
    void Append(const Buffer& buff);

    ssize_t ReadFromFd(int fd, int* errno_); // add new data from file
    ssize_t WriteIntoFd(int fd, int* errno_); // read data into file
};

#endif //BUFFER_HPP
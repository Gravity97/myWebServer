/*
 * @author: Zimo Li
 * @date: 2024-5-17
*/

#include "buffer.h"
#include <assert.h>
#include <cstring>
#include <sys/uio.h>
#include <unistd.h>

void Buffer::MakeSpace(std::size_t len)
{
    if(WritableBytes() + PreinsertableBytes() < len){
        buffer.resize(writePos + len + 1);
    } else {
        auto readableBytes = ReadableBytes();
        std::copy(buffer.begin() + readPos, buffer.begin() + writePos, buffer.begin());
        readPos = 0;
        writePos = readPos + readableBytes;
    }
}

std::size_t Buffer::ReadableBytes() const
{
    return writePos - readPos;
}

std::size_t Buffer::WritableBytes() const
{
    return buffer.size() - writePos;
}

std::size_t Buffer::PreinsertableBytes() const
{
    return readPos;
}

const char* Buffer::ReadPosition() const
{
    return buffer.data() + readPos;
}

const char* Buffer::WritePosition() const
{
    return buffer.data() + writePos;
}

void Buffer::EnsureWritable(std::size_t len)
{
    if(WritableBytes() < len){
        MakeSpace(len);
    }
    assert(WritableBytes() >= len);
}

void Buffer::HasRead(std::size_t len)
{
    readPos += len;
}

void Buffer::HasWritten(std::size_t len)
{
    writePos += len;
}

void Buffer::Retrieve(std::size_t len)
{
    assert(ReadableBytes() <= len);
    readPos += len;
}

void Buffer::RetrieveUntil(const char* end)
{
    assert(ReadPosition() <= end);
    Retrieve(end - ReadPosition());
}

void Buffer::RetrieveAll()
{
    memset(buffer.data(), 0, buffer.size());
    readPos = 0;
    writePos = 0;
}

std::string Buffer::RetrieveAllToStr()
{
    std::string str(ReadPosition(), WritePosition());
    RetrieveAll();
    return str;
}

void Buffer::Append(const char* str, size_t len)
{
    assert(str);
    EnsureWritable(len);
    std::copy(str, str + len, WritePosition());
    HasWritten(len);
}

void Buffer::Append(const std::string& str)
{
    Append(str.data(), str.size());
}

void Buffer::Append(const void* data, size_t len)
{
    Append(static_cast<const char*>(data), len);
}

void Buffer::Append(const Buffer& buff)
{
    Append(buff.ReadPosition(), buff.ReadableBytes());
}

ssize_t Buffer::ReadFromFd(int fd, int* errno_)
{
    char buffer_[65536];
    iovec iov[2];
    auto writableBytes = WritableBytes();

    // prepare a temp buffer for data that can't be read into inner buffer directly.
    iov[0].iov_base = buffer.data();
    iov[0].iov_len = writableBytes;
    iov[1].iov_base = buffer_;
    iov[1].iov_len = sizeof(buffer_);

    const auto len = readv(fd, iov, 2);
    if(len < 0) {
        *errno_ = errno;
    } else if(static_cast<std::size_t>(len) <= writableBytes) {
        HasWritten(static_cast<std::size_t>(len));
    } else {
        HasWritten(writableBytes);
        Append(buffer_, static_cast<std::size_t>(len) - writableBytes);
    }
    return len;
}

ssize_t Buffer::WriteIntoFd(int fd, int* errno_)
{
    auto len = write(fd, ReadPosition(), ReadableBytes());
    if(len < 0) {
        *errno_ = errno;
    } else {
        HasRead(static_cast<std::size_t>(len));
    }
    return len;
}

/*
 * @author: Zimo Li
 * @date: 2024-5-16
*/

#ifndef HTTPCONN_HPP
#define HTTPCONN_HPP

#include <arpa/inet.h>
#include <sys/uio.h>
#include "../buffer/buffer.hpp"
#include "httprequest.hpp"
#include "httpresponse.hpp"

class HttpConn {
private:
    int fd_;
    sockaddr_in addr_;

    bool isClosed_;

    int iovCount_;
    iovec iov_[2]; // iov[0] equals to write buffer, iov[1] equals temp space

    Buffer readBuffer_;
    Buffer writeBuffer_;

    HttpRequest request_;
    HttpResponse response_;

public:
    HttpConn();
    ~HttpConn(); // close http conn

    void init(int sockFd, const sockaddr_in& addr); // new user comes, init everything

    ssize_t read(int* errno_); // if ET(±ßÔµ´¥·¢) mode, read until there is no data, otherwise read only once.
    ssize_t write(int* errno_); // same as above, but if ET or has more than 10240 bytes to write

    bool process(); // parse request and yield response and fill iov[0](write buff), iov[1](file)

    int GetFd() const;
    int GetPort() const;
    const char* GetIP() const; // get string format IP
    sockaddr_in GetAddr() const;

    void close();
};

#endif // HTTPCONN_HPP
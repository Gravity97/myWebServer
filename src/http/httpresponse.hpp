/*
 * @author: Zimo Li
 * @date: 2024-5-25
*/

#ifndef HTTPRESPONSE_HPP
#define HTTPRESPONSE_HPP

#include <string>
#include <sys/stat.h>
#include <unordered_map>
#include "../buffer/buffer.hpp"

class HttpResponse {
private:
    int code_; // status code
    bool isKeepAlive_;

    std::string path_;
    std::string srcDir_;

    char* mmFile_; // file itself
    struct stat mmFileState_; // file state

    static const std::unordered_map<std::string, std::string> SUFFIX_TYPE;
    static const std::unordered_map<int, std::string> CODE_STATUS;
    static const std::unordered_map<int, std::string> CODE_PATH;

    void ChangeToErrorHtml(); // if state is error, change file to error page

    void AddStateLine(Buffer& buffer); // add state line
    void AddHeader(Buffer& buffer); // add header(Connection: keep-alive: Content-type:)
    void AddContent(Buffer& buffer); // just add file length to header, but why?

    std::string GetFileType() const; // get file's type through suffix

public:
    HttpResponse(); // normally init values = 0
    ~HttpResponse();

    void init(const std::string& srcDir, std::string& path,
              bool isKeepAlive = false, int code = -1); // init values and unmap file if it exists.

    char* file(); // get file
    size_t FileLength() const; // get file length
    void ErrorContent(Buffer& buffer, std::string message); // create a error html page and add into buffer
    int code() const; // get status code

    void MakeResponse(Buffer& buffer);
    void UnmapFIle(); // unmap file
};

#endif //HTTPRESPONSE_HPP
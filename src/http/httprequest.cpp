/*
 * @author: Zimo Li
 * @date: 2024-5-24
*/

#include "httprequest.hpp"
#include <regex>
#include "../log/log.hpp"
#include <cassert>
using namespace std;

const unordered_set<string> HttpRequest::DEFAULT_HTML{
            "/index", "/register", "/login",
             "/welcome", "/video", "/picture", };

const unordered_map<string, int> HttpRequest::DEFAULT_HTML_TAG {
            {"/register.html", 0}, {"/login.html", 1},  };

HttpRequest::HttpRequest() : state(REQUEST_LINE), method_(""),
                             path_(""), version_(""), body_("")
{
    header.clear();
    body_.clear();
}

bool HttpRequest::parse(Buffer& buffer)
{
    if(buffer.ReadableBytes() <= 0) return false;

    const char CRLF[] = "\r\n";
    while(buffer.ReadableBytes() > 0 && state != FINISH) {
        const char* lineEnd = search(buffer.ReadPosition(), buffer.WritePositionConst(), CRLF, CRLF + 2);
        string line(buffer.ReadPosition(), lineEnd);

        switch(state) {
        case REQUEST_LINE:
            if(!ParseRequestLine(line)) return false;
            ParsePath();
            break;
        case HEADER:
            ParseHeader(line);
            if(buffer.ReadableBytes() < 2) state = FINISH;
            break;
        case BODY:
            ParseBody(line);
            break;
        default: break;
        }

        if(lineEnd == buffer.WritePositionConst()) break;
        buffer.RetrieveUntil(lineEnd + 2);
    }

    LOG_DEBUG("[%s], [%s], [%s]", method_.c_str(), path_.c_str(), version_.c_str());
    return true;
}

string HttpRequest::path() const
{
    return path_;
}

string& HttpRequest::path()
{
    return path_;
}

string HttpRequest::method() const
{
    return method_;
}

string HttpRequest::version() const
{
    return version_;
}

string HttpRequest::GetPost(const string& key) const
{
    assert(key != "");
    if(post.count(key) == 1) return post.find(key)->second;
    return "";
}

string HttpRequest::GetPost(const char* key) const
{
    assert(key != nullptr);
    if(post.count(key) == 1) return post.find(key)->second;
    return "";
}

bool HttpRequest::IsKeepAlive() const
{
    if(header.count("Connection") == 1) {
        return header.find("Connection")->second == "keep-alive" &&
               version_ == "1.1";
    }
    return false;
}

bool HttpRequest::ParseRequestLine(const string& line)
{
    regex pattern("^([^ ]*) ([^ ]*) HTTP/([^ ]*)$");
    smatch match;
    if(regex_match(line, match, pattern)) {
        method_ = match[1];
        path_ = match[2];
        version_ = match[3];
        state = HEADER; // switch state
        return true;
    }

    LOG_ERROR("RequestLine Error");
    return false;
}

void HttpRequest::ParseHeader(const std::string& line)
{
    regex pattern("^([^:]*): ?(.*)$");
    smatch match;
    if(regex_match(line, match, pattern)) {
        header[match[1]] = match[2];
    } else {
        state = BODY; // now is empty line
    }
}

void HttpRequest::ParseBody(const std::string& line)
{
    body_ = line;
    ParsePost();
    state = FINISH;
    LOG_DEBUG("Body:%s, len:%d", line.c_str(), line.size());
}

void HttpRequest::ParsePath()
{
    if(path_ == "/") {
        path_ = "/index.html";
    } else {
        for(auto &item : DEFAULT_HTML) {
            if(item == path_) {
                path_ += ".html";
                break;
            }
        }
    }
}

void HttpRequest::ParsePost()
{
    if(method_ == "POST" &&
       header["Content-Type"] == "application/x-www-form-urlencoded") {
        ParseEncodedURL();

    }
}

void HttpRequest::ParseEncodedURL()
{
    if(body_.size() == 0) return;

    string key, value;
    int num = 0;
    int n = body_.size();
    int i = 0, j = 0;

    for(; i < n; i++) {
        char ch = body_[i];

        switch (ch) {
        case '=': // the latter is key and former is value
            key = body_.substr(j, i - j);
            j = i + 1;
            break;
        case '+': // old standard see '+' as whitespace
            body_[i] = ' ';
            break;
        case '%': // URL encoding
            assert(i + 2 < n);
            num = ConvertHexToDec(body_[i + 1]) * 16 + ConvertHexToDec(body_[i + 2]);
            body_[i + 1] = static_cast<char>(num / 10) + '0';
            body_[i + 2] = static_cast<char>(num % 10) + '0';
            i += 2;
            break;
        case '&': // end of the key-value pair
            value = body_.substr(j, i - j);
            j = i + 1;
            post[key] = value;
            LOG_DEBUG("%s = %s", key.c_str(), value.c_str());
            break;
        default: break;
        }
    }

    assert(j <= i);
    if(post.count(key) == 0 && j < i) {
        value = body_.substr(j, i - j);
        post[key] = value;
    }
}

int HttpRequest::ConvertHexToDec(char ch)
{
    if(ch >= 'A' && ch <= 'F') return static_cast<int>(ch - 'A') + 10;
    if(ch >= 'a' && ch <= 'f') return static_cast<int>(ch - 'a') + 10;
    return static_cast<int>(ch);
}

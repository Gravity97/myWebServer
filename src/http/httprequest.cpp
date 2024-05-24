/*
 * @author: Zimo Li
 * @date: 2024-5-24
*/

#include "httprequest.hpp"
#include <regex>
#include "../log/log.hpp"
using namespace std;

HttpRequest::HttpRequest() : state(REQUEST_LINE), method(""),
                             path(""), version(""), body("")
{
    head.clear();
    body.clear();
}

bool HttpRequest::ParseRequestLine(const string& line)
{
    regex pattern("^([^ ]*) ([^ ]*) HTTP/([^ ]*)$");
    smatch match;
    if(regex_match(line, match, pattern)) {
        method = match[1];
        path = match[2];
        version = match[3];
        state = HEADER;
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
        head[match[1]] = match[2];
    } else {
        state = BODY;
    }
}

void HttpRequest::ParseBody(const std::string& line)
{
    body = line;

}

void HttpRequest::ParsePath()
{
    if(path == "/") {
        path = "/index.html";
    } else {
        for(auto &item : DEFAULT_HTML) {
            if(item == path) {
                path += ".html";
                break;
            }
        }
    }
}


/*
 * @author: Zimo Li
 * @date: 2024-5-20
*/

#ifndef HTTPREQUEST_HPP
#define HTTPREQUEST_HPP

#include <string>
#include <unordered_map>
#include <unordered_set>

class HttpRequest {
private:
    enum PARSE_STATE {
        REQUEST_LINE,
        HEADER,
        EMPTY_LINE,
        BODY,
    };

    enum HTTP_CODE {
        NO_REQUEST,
        GET_REQUEST,
        BAD_REQUEST,
        NO_RESOURCE,
        FORBIDDENT_REQUEST,
        FILE_REQUEST,
        INTERNAL_ERROR,
        CLOSED_CONNECTION,
    };

    PARSE_STATE state;
    std::string method, path, version, body;
    std::unordered_map<std::string, std::string> head;
    std::unordered_map<std::string, std::string> post;

    static const std::unordered_set<std::string> DEFAULT_HTML;
    static const std::unordered_map<std::string, int> DEFAULT_HTML_TAG;

    bool ParseRequestLine(const std::string& line);
    void ParseHeader(const std::string& line);
    void ParseBody(const std::string& line);
    void ParsePath();
    void ParsePost();

public:
    HttpRequest();
    ~HttpRequest() = default;
};

#endif //HTTPREQUEST_HPP
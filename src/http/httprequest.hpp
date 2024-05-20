/*
 * @author: Zimo Li
 * @date: 2024-5-20
*/

#ifndef HTTPREQUEST_HPP
#define HTTPREQUEST_HPP

class HttpRequest {
private:
    enum PARSE_STATE {
        REQUEST_LINE,
        HEADER,
        EMPTY_LINE,
        BODY
    };

    

};

#endif //HTTPREQUEST_HPP
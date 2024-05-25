/*
 * @author: Zimo Li
 * @date: 2024-5-16
*/

#ifndef SQLCONNRAII_HPP
#define SQLCONNRAII_HPP

#include <mysql/mysql.h>
#include "sqlconnpool.hpp"

class SqlConnRAII {
private:
    MYSQL* sql_;
    SqlConnPool* connpool_;

public:
    SqlConnRAII(MYSQL** sql, SqlConnPool* connpool); // get a conn
    ~SqlConnRAII(); // free sql conn

};

#endif // SQLCONNRAII_HPP
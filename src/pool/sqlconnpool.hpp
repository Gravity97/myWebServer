/*
 * @author: Zimo Li
 * @date: 2024-5-16
*/

#ifndef SQLCONNPOOL_HPP
#define SQLCONNPOOL_HPP

#include <mysql/mysql.h>
#include <queue>
#include <mutex>
#include <semaphore.h>


class SqlConnPool {
private:
    std::queue<MYSQL*> connQueue; // database queue
    std::mutex mtx;
    sem_t sem;

    int MAXCONN;
    int useCount;
    int freeCount;

    SqlConnPool();
    ~SqlConnPool();

};

#endif //SQLCONNPOOL_HPP
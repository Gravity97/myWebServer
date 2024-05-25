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
    static SqlConnPool instance_;

    std::queue<MYSQL*> connQueue; // available database queue
    mutable std::mutex mtx;
    mutable sem_t sem; // PV semaphore

    int MAXCONN;
    int usedCount; // conn number that be used
    int availCount; // conn number that is free

    SqlConnPool(); // init conn num for 0
    ~SqlConnPool(); // close pool

public:
    static SqlConnPool& Instance(); // get instance

    void init(const char* host, int post, const char* user,
              const char* pwd, const char* db, int connSize = 10); // add connSize conns into queue

    MYSQL* GetConn(); // get a conn from queue, using PV and lock protecting
    void FreeConn(MYSQL* conn); // add a conn into queue

    int GetAvailConnCount() const;

    void ClosePool();
};

#endif //SQLCONNPOOL_HPP
#ifndef SINGLE_PATTERN_H
#define SINGLE_PATTERN_H
#include <vector>
#include <mutex>
#include <string>
#include <map>
#include <memory>
#include <stdexcept>
#include <iostream>
#include <cstdlib>
#include <mysql_connection.h>
#include <cppconn/exception.h>
#include <cppconn/resultset.h>
#include <cppconn/prepared_statement.h>

class MySQLConnectionPool {
private:

    MySQLConnectionPool(const MySQLConnectionPool&) = delete;
    MySQLConnectionPool& operator=(const MySQLConnectionPool&) = delete;

    static std::unique_ptr<MySQLConnectionPool> instance_; // 单例唯一实例
    static std::mutex instanceMutex_; // 互斥锁保护实例创建

    // 连接池成员
    std::vector<sql::Connection*> connectionPool; // 连接池
    std::mutex poolMutex_; // 互斥锁
    sql::Driver* driver; // MySQL驱动
    std::string host;
    std::string user;
    std::string password;
    std::string database;
    unsigned int poolSize; // 连接池连接数
    unsigned int freeConnections; // 空闲连接数

    // 初始化配置
    void initConfig(const std::string& configFile);
    void initPool(); // 初始化连接池

public:
    // 获取单例实例
    static MySQLConnectionPool* getInstance();
    // 从连接池获取连接
    sql::Connection* getConnection();
    // 归还连接到连接池
    void releaseConnection(sql::Connection* conn);
};

#endif // SINGLE_PATTERN_H
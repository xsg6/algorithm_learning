#include "single.pattern.h"
#include <iostream>
#include "third_party/json.hpp"
#include <fstream>
#include <stdexcept>

// 单例实例指针（使用 unique_ptr 管理生命周期）
std::unique_ptr<MySQLConnectionPool> MySQLConnectionPool::instance_ = nullptr;
// 用于保护单例实例创建的互斥量
std::mutex MySQLConnectionPool::instanceMutex_;
// 初始化连接池（禁止外部new）
MySQLConnectionPool::MySQLConnectionPool(){
        try{
            initConfig("database.json");
            initPool();
        }catch(const std::exception& e){
            std::cerr << "Error initializing MySQLConnectionPool: " << e.what() << std::endl;
            std::exit(1);
        }
    }
/*
 * 获取单例实例
 * 说明：
 * - 采用所谓的“双重检查”模式：先检查 instance_，若为空则加锁并再次检查后创建实例。
 * - 注意：在 C++ 中，双重检查若不使用原子操作/内存屏障或 std::call_once 可能存在线程安全问题。
 *   更稳妥的做法是使用 std::call_once 或在函数内使用静态局部变量（C++11 起线程安全）。
 */
MySQLConnectionPool* MySQLConnectionPool::getInstance() {
    if (!instance_) {
        std::lock_guard<std::mutex> lock(instanceMutex_);
        if (!instance_) {
            instance_.reset(new MySQLConnectionPool());
        }
    }
    return instance_.get();
}

/*
 * 从连接池获取一个可用的连接
 * - 使用 poolMutex_ 保护连接容器的并发访问
 * - 若没有可用连接，抛出 runtime_error
 */
sql::Connection* MySQLConnectionPool::getConnection() {
    std::lock_guard<std::mutex> lock(poolMutex_);
    if (connectionPool.empty()) {
        throw std::runtime_error("No available connections in the pool");
    }
    sql::Connection* conn = connectionPool.back();
    connectionPool.pop_back();
    --freeConnections;
    return conn;
}

/*
 * 释放连接回连接池
 * - 将连接放回容器并增加空闲计数
 * - 需要保证调用方提供的 conn 是有效且来自本池
 */
void MySQLConnectionPool::releaseConnection(sql::Connection* conn) {
    std::lock_guard<std::mutex> lock(poolMutex_);
    connectionPool.push_back(conn);
    ++freeConnections;
}

/*
 * 从 JSON 配置文件加载数据库连接配置
 * - configFile: 配置文件路径（JSON 格式）
 * - 期望字段：host, user, password, database, port, pool_size
 * - 若文件打开失败或解析失败，抛出 runtime_error
 */
void MySQLConnectionPool::initConfig(const std::string& configFile) {
    std::ifstream file(configFile);
    if (!file.is_open()) {
        throw std::runtime_error("Could not open config file: " + configFile);
    }
    nlohmann::json configJson;
    try {
        file >> configJson;
        host = configJson["host"].get<std::string>();
        user = configJson["user"].get<std::string>();
        password = configJson["password"].get<std::string>();
        database = configJson["database"].get<std::string>();
        port = configJson["port"].get<unsigned int>();
        poolSize = configJson["pool_size"].get<unsigned int>();
    } catch (const nlohmann::json::exception& e) {
        throw std::runtime_error("Error parsing config file: " + std::string(e.what()));
    }
}

/*
 * 根据已加载的配置初始化连接池
 * - 获取 MySQL 驱动并创建 poolSize 个连接
 * - 创建连接后设置默认 schema（数据库）
 * - 将连接放入 connectionPool 并设置 freeConnections
 */
void MySQLConnectionPool::initPool() {
    driver = sql::mysql::get_mysql_driver_instance();
    for (unsigned int i = 0; i < poolSize; ++i) {
        sql::Connection* conn = driver->connect(host, user, password);
        conn->setSchema(database);
        connectionPool.push_back(conn);
    }
    freeConnections = poolSize;
}

/*
 * 示例 main：获取单例连接池、拿取一个连接并释放
 * - 真实使用中应先调用 initConfig 和 initPool 进行初始化
 * - 这里仅演示基本用法与异常处理
 */
int main() {
    try {
        MySQLConnectionPool* pool = MySQLConnectionPool::getInstance();
        // 注意：示例未调用 initConfig / initPool，真实场景请先初始化
        sql::Connection* conn = pool->getConnection();
        // 使用连接执行数据库操作（此处省略具体操作）
        pool->releaseConnection(conn);
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }
    return 0;
}
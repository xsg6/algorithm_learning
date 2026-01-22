#ifndef LOGGER_HPP
#define LOGGER_HPP
#include<thread>
#include <fstream>
#include <mutex>
#include <ctime>
#include "blockQueue.hpp"
enum class LogLevel {
    DEBUG,
    INFO,
    WARNING,
    ERROR
};

class Logger {
private:
    std::ofstream m_logFile;
    std::mutex m_mutex;
    LogLevel m_logLevel;
    bool m_isAsync;//是否异步
    BlockQueue<std::string>*m_logQueue;
    std::thread* m_writeThread;//后台异步写线程
    Logger();
    ~Logger();
    //后台写线程循环写
    void asyncWriteLog();
    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;
    std::string getCurrentTime();
    std::string levelToString(LogLevel level);

public:
    static Logger& getInstance();
    void init(const std::string& logFile = "webserver.log");
    
    void log(LogLevel level, const std::string& message);
// 便捷函数
    void debug(const std::string& msg) { log(LogLevel::DEBUG, msg); }
    void info(const std::string& msg) { log(LogLevel::INFO, msg); }
    void warning(const std::string& msg) { log(LogLevel::WARNING, msg); }
    void error(const std::string& msg) { log(LogLevel::ERROR, msg); }
    
    
    

};

#endif // LOGGER_HPP
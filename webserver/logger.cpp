#include "logger.hpp"
#include <iomanip>
#include <sstream>
#include<iostream>

Logger::Logger() : m_logLevel(LogLevel::INFO), m_isAsync(false),m_logQueue(nullptr),m_writeThread(nullptr) {}
Logger::~Logger(){
    if(m_writeThread&&m_writeThread->joinable()){//joinable是检查线程是否可以join
        //先关队列，再等线程结束
        while(!m_logQueue->empty()){
            m_logQueue->flush();
        }
        m_logQueue->close();
        m_writeThread->join();//等待线程结束
    }
    if(m_logFile.is_open()){
        m_logFile.flush();
        m_logFile.close();
    }
    if (m_writeThread) {
        delete m_writeThread;
        m_writeThread = nullptr;
    }
    if (m_logQueue) {
        delete m_logQueue;
        m_logQueue = nullptr;
    }
}

Logger& Logger::getInstance() {
    static Logger instance;
    return instance;
}

void Logger::init(const std::string& logFile) {
    if (m_logQueue || m_writeThread) return;
    m_logQueue=new BlockQueue<std::string>(1000);
    m_logFile.open(logFile, std::ios::app);
    if(!m_logFile.is_open()){
        std::cout<<"open log file failed"<<std::endl;
        return ;
    }
    m_isAsync=true;
    //启动后台写线程
    m_writeThread=new std::thread(&Logger::asyncWriteLog, this);
}
//消费者线程
void Logger::asyncWriteLog() {
    std::string singleLog;
    //队列有文件就写
    while(m_logQueue->pop(singleLog)){
        if(m_logFile.is_open()){
            m_logFile<<singleLog;
            m_logFile.flush();
        }
    } 
}
//生产者线程
void Logger::log(LogLevel level, const std::string& message) { 
    if(level<m_logLevel)return ;
    //拼接日志
    std::ostringstream oss;
    oss<<"["<<getCurrentTime()<<"]["<<levelToString(level)<<"]"<<message<<"\n";
    std::string logEntry=oss.str();
    //异步且日志队列非空
    if(m_isAsync&&m_logQueue){
        m_logQueue->push(logEntry);
    }
    else{
        std::lock_guard<std::mutex> lock(m_mutex);
        std::cout<<logEntry;
    }
}
std::string Logger::getCurrentTime() {
    std::time_t now = std::time(nullptr);
    std::tm tm;
    localtime_r(&now,&tm);
    std::stringstream ss;
    ss << std::put_time(&tm, "%Y-%m-%d %H:%M:%S");
    return ss.str();
}

std::string Logger::levelToString(LogLevel level) {
    switch (level) {
        case LogLevel::DEBUG:
            return "DEBUG";
        case LogLevel::INFO:
            return "INFO";
        case LogLevel::WARNING:
            return "WARNING";
        case LogLevel::ERROR:
            return "ERROR";
        default:
            return "UNKNOWN";
    }
}
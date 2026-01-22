#ifndef blockQueue_hpp
#define blockQueue_hpp 
#include <mutex>
#include <condition_variable>//提供条件变量
#include <deque>
#include<sys/time.h>
template<typename T>
class BlockQueue { 
private:
    std::deque<T> m_queue;
    std::mutex m_mutex;
    size_t m_capacity;
    std::condition_variable m_cond_producer;
    std::condition_variable m_cond_consumer;
    bool m_is_closed;
public:
    explicit BlockQueue(size_t max_capacity=1000):m_capacity(max_capacity){
        m_is_closed=false;
    }
    ~BlockQueue(){
        close();
    }
    void close(){
        {
        std::unique_lock<std::mutex>locker(m_mutex);
        m_is_closed=true;//加锁避免修改出问题
        }
        m_cond_producer.notify_all();
        m_cond_consumer.notify_all();
    }
    // 生产者
    bool push(const T& item){
        std::unique_lock<std::mutex>locker(m_mutex);
        while(m_queue.size()>=m_capacity){
            m_cond_producer.wait(locker);
            if(m_is_closed)return false;
        }
        m_queue.push_back(item);
        m_cond_consumer.notify_one();//唤醒一个消费者
        return true;
    }
    // 消费者
    bool pop(T& item){
        std::unique_lock<std::mutex>locker(m_mutex);
        while(m_queue.empty()){
            m_cond_consumer.wait(locker);//等待，释放锁，被唤醒，再获取锁
            if(m_is_closed)return false;
        }
        item = m_queue.front();
        m_queue.pop_front();
        m_cond_producer.notify_one();//唤醒一个生产者
        return true;
    }
    bool empty(){
        std::unique_lock<std::mutex>locker(m_mutex);
        return m_queue.empty();
    }
    void flush(){
        m_cond_consumer.notify_all();
    }
};
#endif
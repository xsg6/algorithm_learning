#include "simple_thread_pool.hpp"

// [MOD] getInstance：给 threadNum 做 fallback，避免 0 线程
SimpleThreadPool& SimpleThreadPool::getInstance(size_t threadNum) {
    if (threadNum == 0) {
        threadNum = std::thread::hardware_concurrency();//hardware_concurrency获取线程（核心数）
        if (threadNum == 0) threadNum = 4; // fallback
    }
    static SimpleThreadPool instance(threadNum);
    return instance;
}

SimpleThreadPool::SimpleThreadPool(size_t threadNum)
    : m_stop(false) {
    m_workers.reserve(threadNum);

    for (size_t i = 0; i < threadNum; ++i) {
        m_workers.emplace_back([this]() {
            while (true) {
                std::function<void()> task;

                {
                    std::unique_lock<std::mutex> lock(m_mtx);
                    m_cv.wait(lock, [this]() {
                        return m_stop || !m_tasks.empty();
                    });

                    if (m_stop && m_tasks.empty()) {
                        return;
                    }

                    task = std::move(m_tasks.front());
                    m_tasks.pop();
                }

                task();
            }
        });
    }
}

SimpleThreadPool::~SimpleThreadPool() {
    {
        std::unique_lock<std::mutex> lock(m_mtx);
        m_stop = true;
    }

    m_cv.notify_all();

    for (std::thread& worker : m_workers) {
        if (worker.joinable()) {
            worker.join();
        }
    }
}
#ifndef SIMPLE_THREAD_POOL_HPP
#define SIMPLE_THREAD_POOL_HPP

#include <vector>
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <functional>
#include <future>
#include <atomic>
#include <stdexcept>
#include <type_traits>   // [MOD] invoke_result_t
#include <utility>

class SimpleThreadPool {
public:
    // [MOD] hardware_concurrency() 可能返回 0，所以做 fallback
    static SimpleThreadPool& getInstance(size_t threadNum = 0);

    // ===================== 原 submit（保留：需要 future 的场景） =====================
    template<typename Callable, typename... Arguments>
    auto submit(Callable&& task, Arguments&&... args) {
        using TaskReturnType = std::invoke_result_t<Callable, Arguments...>;

        auto packagedTask = std::make_shared<std::packaged_task<TaskReturnType()>>(
            std::bind(std::forward<Callable>(task), std::forward<Arguments>(args)...)
        );
        auto resultFuture = packagedTask->get_future();

        {
            std::unique_lock<std::mutex> lock(m_mtx);
            if (m_stop) {
                throw std::runtime_error("Cannot submit task to stopped thread pool");
            }
            m_tasks.emplace([packagedTask]() { (*packagedTask)(); });
        }

        m_cv.notify_one();
        return resultFuture;
    }

    // ===================== [MOD] 新增：轻量 post（无 future，适合网络事件） =====================
    // 为什么：webserver 的任务一般不需要返回值，packaged_task/future 会带来额外分配和开销
    template<typename F>
    void post(F&& f) {
        {
            std::unique_lock<std::mutex> lock(m_mtx);
            if (m_stop) {
                throw std::runtime_error("Cannot post task to stopped thread pool");
            }
            m_tasks.emplace(std::forward<F>(f)); // std::function<void()> 接住任务
        }
        m_cv.notify_one();
    }

    ~SimpleThreadPool();

    SimpleThreadPool(const SimpleThreadPool&) = delete;
    SimpleThreadPool& operator=(const SimpleThreadPool&) = delete;
    SimpleThreadPool(SimpleThreadPool&&) = delete;
    SimpleThreadPool& operator=(SimpleThreadPool&&) = delete;

private:
    explicit SimpleThreadPool(size_t threadNum);

    std::vector<std::thread> m_workers;
    std::queue<std::function<void()>> m_tasks;
    std::mutex m_mtx;
    std::condition_variable m_cv;
    std::atomic<bool> m_stop;
};

#endif // SIMPLE_THREAD_POOL_HPP
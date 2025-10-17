#ifndef THREAD_POOL_HPP
#define THREAD_POOL_HPP

#include <vector>
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <functional>
#include <future>
#include <memory>
#include <stdexcept>

class ThreadPool
{
public:
    explicit ThreadPool(size_t numThreads = 0)
    {
        if (numThreads == 0)
        {
            numThreads = std::thread::hardware_concurrency();
            if (numThreads == 0)
                numThreads = 4;
        }
        workers_.reserve(numThreads);

        for (size_t i = 0; i < numThreads; ++i)
            workers_.emplace_back([this] { workerLoop_(); });
    }

    ~ThreadPool() noexcept
    {
        {
            std::scoped_lock lock(mutex_);
            stop_ = true;
        }
        cv_.notify_all();

        for (auto& worker : workers_)
        {
            if (worker.joinable())
                worker.join();
        }
    }

    ThreadPool(const ThreadPool&) = delete;
    ThreadPool &operator=(const ThreadPool&) = delete;
    ThreadPool(ThreadPool&&) = delete;
    ThreadPool &operator=(ThreadPool&&) = delete;

    template<typename F, typename... Args>
    auto enqueue(F&& f, Args&&... args) -> std::future<std::invoke_result_t<F, Args...>>
    {
        using ReturnType = std::invoke_result_t<F, Args...>;

        auto task = std::make_shared<std::packaged_task<ReturnType()>>(
            [f = std::forward<F>(f), ...args = std::forward<Args>(args)]() mutable {
                return std::invoke(std::forward<F>(f), std::forward<Args>(args)...);
            }
        );

        auto future = task->get_future();
        {
            std::scoped_lock lock(mutex_);
            if (stop_)
                throw std::runtime_error("Cannot enqueue on stopped thread pool");

            queue_.emplace([task = std::move(task)]() { (*task)(); });
        }

        cv_.notify_one();

        return future;
    }

    size_t size() const noexcept
    {
        return workers_.size();
    }

    size_t pendingTasks() const
    {
        std::scoped_lock lock(mutex_);
        return queue_.size();
    }

private:
    std::vector<std::thread> workers_;
    std::queue<std::function<void()>> queue_;
    mutable std::mutex mutex_;
    std::condition_variable cv_;
    bool stop_ = false;

    void workerLoop_()
    {
        while (true)
        {
            std::function<void()> task;

            {
                std::unique_lock<std::mutex> lock(mutex_);

                cv_.wait(lock, [this] { return !queue_.empty() || stop_; });

                if (stop_ && queue_.empty())
                    return;

                if (!queue_.empty())
                {
                    task = std::move(queue_.front());
                    queue_.pop();
                }
            }
            if (task)
                task();
        }
    }
};

#endif
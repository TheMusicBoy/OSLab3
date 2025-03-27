#include <atomic>
#include <common/threadpool.h>

namespace NCommon {

////////////////////////////////////////////////////////////////////////////////

TThreadPool::TThreadPool(size_t numThreads)
    : stop_(false)
{
    for (size_t i = 0; i < numThreads; ++i) {
        workers_.emplace_back(&TThreadPool::Worker, this);
    }
}

TThreadPool::~TThreadPool() {
    {
        std::unique_lock<std::mutex> lock(queue_mutex_);
        stop_.store(true, std::memory_order_release);
    }
    cv_.notify_all();
    for (std::thread& worker : workers_) {
        worker.join();
    }
}

void TThreadPool::Worker() {
    while (!stop_.load(std::memory_order_acquire)) {
        std::function<void()> task;
        {
            std::unique_lock<std::mutex> lock(queue_mutex_);
            cv_.wait(lock, [this] { return stop_.load(std::memory_order_acquire) || !tasks_.empty(); });
            if (stop_.load(std::memory_order_acquire) && tasks_.empty()) {
                return;
            }
            task = std::move(tasks_.front());
            tasks_.pop();
        }
        task();
    }
}

////////////////////////////////////////////////////////////////////////////////

} // namespace NCommon

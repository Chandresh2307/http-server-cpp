#include "thread_pool.h"

ThreadPool::ThreadPool(size_t num_threads) {
    if (num_threads == 0)
        throw std::invalid_argument("ThreadPool: num_threads must be > 0");

    workers_.reserve(num_threads);
    for (size_t i = 0; i < num_threads; ++i)
        workers_.emplace_back(&ThreadPool::worker_loop, this);
}

ThreadPool::~ThreadPool() {
    // Signal all workers to drain and exit.
    stop_.store(true, std::memory_order_relaxed);
    condition_.notify_all();

    for (auto& t : workers_)
        if (t.joinable()) t.join();
}

void ThreadPool::enqueue(std::function<void()> task) {
    {
        std::lock_guard<std::mutex> lock(queue_mutex_);
        tasks_.push(std::move(task));
    }
    condition_.notify_one(); // wake exactly one idle worker
}

size_t ThreadPool::pending() const {
    std::lock_guard<std::mutex> lock(queue_mutex_);
    return tasks_.size();
}

void ThreadPool::worker_loop() {
    while (true) {
        std::function<void()> task;
        {
            std::unique_lock<std::mutex> lock(queue_mutex_);

            // Block until there is work OR we are asked to stop.
            condition_.wait(lock, [this] {
                return !tasks_.empty() || stop_.load(std::memory_order_relaxed);
            });

            // Drain remaining tasks even after stop_ is set.
            if (tasks_.empty()) return;

            task = std::move(tasks_.front());
            tasks_.pop();
        }
        task(); // execute outside the lock
    }
}

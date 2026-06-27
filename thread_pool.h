#pragma once

#include <vector>
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <functional>
#include <atomic>
#include <stdexcept>

/**
 * ThreadPool
 * ----------
 * Fixed-size pool of worker threads that pull tasks from a shared,
 * mutex-protected queue. Workers block on a condition_variable when
 * idle, so they consume zero CPU while waiting.
 *
 * Usage:
 *   ThreadPool pool(4);
 *   pool.enqueue([]{ handle_client(fd); });
 */
class ThreadPool {
public:
    explicit ThreadPool(size_t num_threads);
    ~ThreadPool();

    // Submit a callable to the queue; wakes one idle worker.
    void enqueue(std::function<void()> task);

    // Current pending-task count (approximate).
    size_t pending() const;

    // Disallow copy and move.
    ThreadPool(const ThreadPool&)            = delete;
    ThreadPool& operator=(const ThreadPool&) = delete;

private:
    std::vector<std::thread>          workers_;
    std::queue<std::function<void()>> tasks_;
    mutable std::mutex                queue_mutex_;
    std::condition_variable           condition_;
    std::atomic<bool>                 stop_{false};

    void worker_loop();
};

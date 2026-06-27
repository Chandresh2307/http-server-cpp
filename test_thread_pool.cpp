#include "thread_pool.h"
#include <gtest/gtest.h>
#include <atomic>
#include <chrono>
#include <vector>

// ── Construction ──────────────────────────────────────────────────────────────

TEST(ThreadPoolTest, ConstructsWithValidSize) {
    EXPECT_NO_THROW(ThreadPool pool(4));
}

TEST(ThreadPoolTest, ThrowsOnZeroThreads) {
    EXPECT_THROW(ThreadPool pool(0), std::invalid_argument);
}

// ── Task execution ────────────────────────────────────────────────────────────

TEST(ThreadPoolTest, ExecutesSingleTask) {
    ThreadPool pool(2);
    std::atomic<int> counter{0};

    pool.enqueue([&counter]{ counter.fetch_add(1); });

    // Give worker time to process.
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    EXPECT_EQ(counter.load(), 1);
}

TEST(ThreadPoolTest, ExecutesAllTasksConcurrently) {
    const int TASK_COUNT = 100;
    ThreadPool pool(4);
    std::atomic<int> counter{0};

    for (int i = 0; i < TASK_COUNT; ++i)
        pool.enqueue([&counter]{ counter.fetch_add(1); });

    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    EXPECT_EQ(counter.load(), TASK_COUNT);
}

TEST(ThreadPoolTest, HandlesManyTasksWithFewThreads) {
    ThreadPool pool(2);
    std::atomic<int> counter{0};

    for (int i = 0; i < 50; ++i)
        pool.enqueue([&counter]{ counter.fetch_add(1); });

    std::this_thread::sleep_for(std::chrono::milliseconds(300));
    EXPECT_EQ(counter.load(), 50);
}

TEST(ThreadPoolTest, TasksRunOnDifferentThreads) {
    ThreadPool pool(4);
    std::mutex mtx;
    std::vector<std::thread::id> ids;

    for (int i = 0; i < 8; ++i) {
        pool.enqueue([&mtx, &ids]{
            std::lock_guard<std::mutex> lock(mtx);
            ids.push_back(std::this_thread::get_id());
        });
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    // At least 2 distinct thread IDs should have been used.
    std::sort(ids.begin(), ids.end());
    ids.erase(std::unique(ids.begin(), ids.end()), ids.end());
    EXPECT_GT(ids.size(), 1u);
}

// ── Pending ───────────────────────────────────────────────────────────────────

TEST(ThreadPoolTest, PendingDecreasesAsTasksComplete) {
    ThreadPool pool(1);
    std::atomic<bool> block{true};

    // Fill the queue — first task blocks the single worker.
    pool.enqueue([&block]{ while (block.load()); });
    for (int i = 0; i < 5; ++i)
        pool.enqueue([]{ /* no-op */ });

    std::this_thread::sleep_for(std::chrono::milliseconds(20));
    EXPECT_GT(pool.pending(), 0u); // still tasks queued

    block.store(false); // unblock worker
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    EXPECT_EQ(pool.pending(), 0u);
}

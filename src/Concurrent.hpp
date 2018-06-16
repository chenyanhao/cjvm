//
// Created by cyh on 2018/6/15.
//

#ifndef CJVM_CONCURRENT_H
#define CJVM_CONCURRENT_H

#include <queue>
#include <future>
#include <functional>
#include <thread>
#include <atomic>
#include <mutex>

class SpinLock {
public:
    SpinLock() = default;

    SpinLock(const SpinLock&) = delete; // 不允许拷贝
    SpinLock(SpinLock&&) = delete; // 不允许移动
    SpinLock& operator=(const SpinLock&) = delete; // 不允许拷贝复制
    SpinLock&&operator=(SpinLock&&) = delete; // 不允许移动赋值

    inline void lock() noexcept {
        while (flag.test_and_set(std::memory_order_acquire));
    }

    inline void unlock() noexcept {
        flag.clear(std::memory_order_release);
    }

private:
    std::atomic_flag flag = ATOMIC_FLAG_INIT;
};

class ThreadPool {
public:
    ThreadPool(): done(false) {}
    ~ThreadPool() noexcept;

public:
    virtual void initialize(int startThreadNum) noexcept;
    virtual void runPendingWork();
    template<typename Func> std::future<void> submit(Func task);
    virtual void finalize() { done = true; }

protected:
    std::atomic_bool done{};
    std::vector<std::thread> threads;
    std::queue<std::packaged_task<void()>> taskQueue;
    std::mutex taskQueueMtx;

};


template<typename Func>
std::future<void> ThreadPool::submit(Func task) {
    std::packaged_task<void()> pt(task);
    std::future<void> f = pt.get_future();
    std::lock_guard<std::mutex> lock(taskQueueMtx);
    taskQueue.push(std::move(pt));
    return f;
}


#endif //CJVM_CONCURRENT_H

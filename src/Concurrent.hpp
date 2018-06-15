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

using namespace std;

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


#endif //CJVM_CONCURRENT_H

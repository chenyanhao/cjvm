//
// Created by ha on 18/6/16.
//

#ifndef CJVM_FRAME_H_H
#define CJVM_FRAME_H_H

#include <deque>
#include <mutex>
#include <atomic>
#include "JavaType.h"
#include "Concurrent.hpp"

class JType;

class Frame {
public:
    std::deque<JType*> locals;
    std::deque<JType*> stack;

    ~Frame() {
        while (!locals.empty()) {
            auto *temp = locals.back();
            locals.pop_back();
            delete temp;
        }
        while (!stack.empty()) {
            auto *type = stack.back();
            stack.pop_back();
            delete type;
        }
    }
};

class StackFrames {
public:
    StackFrames() = default;
    ~StackFrames() {
        std::lock_guard<SpinLock> lock(spinMtx);
        while (!frames.empty()) {
            auto temp = frames.back();
            frames.pop_back();
            delete temp;
        }
    }

    inline auto back() noexcept {
        std::lock_guard<SpinLock> lock(spinMtx);
        return frames.back();
    }

    inline void push_back(Frame *f) {
        std::lock_guard<SpinLock> lock(spinMtx);
        frames.push_back(f);
    }

    inline void pop_back() {
        std::lock_guard<SpinLock> lock(spinMtx);
        frames.pop_back();
    }

    inline bool empty() {
        std::lock_guard<SpinLock> lock(spinMtx);
        return frames.empty();
    }

    inline auto cbegin() noexcept {
        std::lock_guard<SpinLock> lock(spinMtx);
        return frames.cbegin();
    }

    inline auto cend() noexcept {
        std::lock_guard<SpinLock> lock(spinMtx);
        return frames.cend();
    }

private:
    std::deque<Frame*> frames;
    SpinLock spinMtx;
};



#endif //CJVM_FRAME_H_H

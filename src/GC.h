//
// Created by cyh on 2018/7/26.
//

#ifndef CJVM_GC_H
#define CJVM_GC_H

#include <unordered_set>
#include <memory>

#include "Option.h"
#include "RuntimeEnv.h"
#include "Concurrent.hpp"

class JType;

enum class GCPolicy {
    GC_MARK_AND_SWEEP
};

class ConcurrentGC {

public:
    ConcurrentGC() : overMemoryThreshold(false), safepointWaitCnt(0) {
        gcThreadPool.initialize(std::thread::hardware_concurrency());
    }

    bool shallGC() const { return overMemoryThreshold; }
    void notifyGC() { safepointWaitCnt = 0; overMemoryThreshold = false; }
    void stopTheWorld();
    void gc(GCPolicy policy = GCPolicy::GC_MARK_AND_SWEEP);

    void terminateGC() { gcThreadPool.finalize(); }

private:
    inline void pushObjectBitmap(size_t offset) {
        objectBitmap.insert(offset);
    }

    void markAndSweep();
    void mark(JType *ref);
    void sweep();
    std::unordered_set<size_t> objectBitmap;
    SpinLock objSpin;


    std::unordered_set<size_t> arrayBitmap;
    SpinLock arrSpin;
    std::atomic_bool overMemoryThreshold;
    std::mutex overMemoryThresholdMtx;

    int safepointWaitCnt;
    std::mutex safepointWaitMtx;
    std::condition_variable safepointWaitCond;



private:
    class GCThreadPool : public ThreadPool {

    };

    GCThreadPool gcThreadPool;
};

class GC {

};


#endif //CJVM_GC_H

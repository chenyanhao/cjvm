//
// Created by ha on 18/6/16.
//

#ifndef CJVM_OBJECTMONITOR_H
#define CJVM_OBJECTMONITOR_H

#include <thread>
#include <condition_variable>
#include <cstdint>

class ObjectMonitor {
public:
    void enter(std::thread::id tid);
    void exit();

private:
    std::mutex internalMtx;
    volatile bool entered = false;
    int32_t monitorCnt = 0;
    std::thread::id owner;
    std::condition_variable cv;
};


#endif //CJVM_OBJECTMONITOR_H

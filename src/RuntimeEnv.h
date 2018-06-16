//
// Created by ha on 18/6/16.
//

#ifndef CJVM_RUNTIMEENV_H
#define CJVM_RUNTIMEENV_H

#include <unordered_map>
#include <string>
#include "Type.h"
#include "Frame.h"

class JType;
class Frame;
class JavaHeap;
class MethodArea;
class ConcurrentGC;

class RuntimeEnv {
public:
    RuntimeEnv();
    ~RuntimeEnv();

    MethodArea *ma;
    JavaHeap *jheap;
    std::unordered_map<std::string, JType*(*)(RuntimeEnv*)> nativeMethods;
    ConcurrentGC *gc;
};

extern RuntimeEnv crt;
extern thread_local StackFrames frames;

#endif //CJVM_RUNTIMEENV_H

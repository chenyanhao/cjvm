//
// Created by ha on 18/6/16.
//

#include <cstdio>
#include <cassert>
#include "JavaException.h"

// TODO;
void StackTrace::printStackTrace() {
    assert(!exceptionStackTrace.empty());
    assert(throwExceptionClass != nullptr);

//    printf("Thrown %s at %s()\n", throwExceptionClass->getClassName(), exceptionStackTrace[0]);
}

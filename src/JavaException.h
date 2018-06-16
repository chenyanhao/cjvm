//
// Created by ha on 18/6/16.
//

#ifndef CJVM_JAVAEXCEPTION_H
#define CJVM_JAVAEXCEPTION_H

#include <vector>
#include <string>
#include "JavaType.h"

class JavaClass;

class StackTrace {
public:
    void printStackTrace();
    void setThrowExceptionInfo(JObject *throwableObject);
    void extendExceptionStackTrace(const char *methodName) {
        exceptionStackTrace.push_back(methodName);
    }

protected:
    std::vector<const char*> exceptionStackTrace;

private:
    const JavaClass *throwExceptionClass = nullptr;
    std::string detailedMsg;
};

class JavaException : public StackTrace {
public:
    bool hasUnhandledException() const {
        return unhandledException;
    }

    void markException() {
        unhandledException = true;
    }

    void sweepException() {
        unhandledException = false;
        exceptionStackTrace.clear();
    }

private:
    volatile bool unhandledException = false;

};


#endif //CJVM_JAVAEXCEPTION_H

//
// Created by ha on 18/6/16.
//

#ifndef CJVM_METHODAREA_H
#define CJVM_METHODAREA_H


#include <unordered_map>
#include <unordered_set>
#include <string>
#include <vector>
#include <algorithm>
#include <cstring>
#include <mutex>
#include "ClassFile.h"

class CodeExecution;
class JavaClass;
class ConcurrentGC;

class MethodArea {
    friend class ConcurrentGC;
public:
    MethodArea(const std::vector<std::string> &libPaths);
    ~MethodArea();
    JavaClass* findJavaClass(const char *javaClassName);
    bool loadJavaClass(const char *javaClassName);
    void linkJavaClass(const char *javaClassName);
    void initJavaClass(CodeExecution &execution, const char *javaClassName);

public:
    JavaClass* loadClassIfAbsent(const char *javaClassName) {
        std::lock_guard<std::recursive_mutex> lockMA(maMutex);
        JavaClass *jc = findJavaClass(javaClassName);
        if (jc) {
            return jc;
        }
        loadJavaClass(javaClassName);
        return findJavaClass(javaClassName);
    }

    void linkClassIfAbsent(const char *javaClassName) {
        std::lock_guard<std::recursive_mutex> lockMA(maMutex);
        bool linked = false;
        for (auto *p : linkedClasses) {
            if (strcmp(p, javaClassName) == 0) {
                linked = true;
            }
        }

        if (!linked) {
            linkJavaClass(javaClassName);
        }
    }


private:
    std::recursive_mutex maMutex;
    std::unordered_set<const char*> linkedClasses;
    std::unordered_set<const char*> initedClasses;
    std::unordered_map<std::string, JavaClass*> classTable;
    std::vector<std::string> searchPaths;

    std::string parseName2Path(const char *name);
};


#endif //CJVM_METHODAREA_H

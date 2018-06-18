//
// Created by ha on 18/6/18.
//

#ifndef CJVM_DEBUG_H
#define CJVM_DEBUG_H

#include "JavaClass.h"
#include <deque>
#include <iostream>
#include <string>
#include <iomanip>
#include <utility>

class JavaClass;


class Inspector {
    static void printConstantPool(const JavaClass &jc);
    static void printClassAccessFlag(const JavaClass &jc);
    static void printField(const JavaClass &jc);
    static void printMethods(const JavaClass &jc);
    static void printJavaClassFileVersion(const JavaClass &jc);
    static void printInterfaces(const JavaClass &jc);
    static void printClassFileAttrs(const JavaClass &jc);

    static void printSizeOfInternalTypes();
    static void printOpCode(u1 *code, u2 index);
};


#endif //CJVM_DEBUG_H

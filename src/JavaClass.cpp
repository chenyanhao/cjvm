//
// Created by ha on 18/6/16.
//

#include <iostream>
#include <vector>
#include <cassert>
#include "JavaClass.h"
#include "MethodArea.h"
#include "Debug.h"

JavaClass::JavaClass(const char *classFilePath) : reader(classFilePath) {
    raw.constPoolInfo = nullptr;
    raw.fields = nullptr;
    raw.methods = nullptr;
    raw.attributes = nullptr;
}

JavaClass::~JavaClass() {
    for (auto &v : sfield) {
        delete v.second;
    }
}

std::vector<u2> JavaClass::getInterfacesIndex() const {
    if (raw.interfacesCount == 0) {
        return std::vector<u2>();
    }

    std::vector<u2> v;
    FOR_EACH(i, raw.interfacesCount) {
        v.push_back(dynamic_cast<CONSTANT_Class*>(raw.constPoolInfo[i])->nameIndex);
    }
    return v;
}

MethodInfo* JavaClass::getMethod(const char *methodName, const char *methodDescriptor) const {
    FOR_EACH(i, raw.methodsCount) {
        assert(typeid(*raw.constPoolInfo[raw.methods[i].nameIndex]) == typeid(CONSTANT_Utf8));

        const char *methodNameIndex = (char*)getString(raw.methods[i].nameIndex);
        const char *methodDescriptor = (char*)getString(raw.methods[i].descriptorIndex);
        if (strcmp(methodNameIndex, methodName) == 0 &&
            strcmp(methodDescriptor, methodDescriptor) == 0) {
            return &raw.methods[i];
        }
    }
    return nullptr;
}

void JavaClass::parseClassFile() {
    int ff = 0;
    // 魔数
    raw.magic = reader.readU4();
    if (raw.magic != JAVA_CLASS_FILE_MAGIC_NUMBER) {
        std::cerr << __func__ << ":Failed to read content from bytecode file\n";
        exit(EXIT_FAILURE);
    }

    // 版本号
    raw.majorVersion = reader.readU2();
    raw.minorVersion = reader.readU2();
#ifdef CJVM_DEBUG_SHOW_VERSION
    Inspector::printClassFileVersion(*this);
#endif
    if (raw.majorVersion < JAVA_6_MAJOR || raw.majorVersion > JAVA_9_MAJOR) {
        std::cerr << __func__ << ":Failed to read content from bytecode file\n";
        exit(EXIT_FAILURE);
    }

    // 常量池
    raw.constPoolCount = reader.readU2();
    if (raw.constPoolCount > 0 && !parseConstantPool(raw.constPoolCount)) {
        std::cerr << __func__ << ":Failed to parse constant pool\n";
        exit(EXIT_FAILURE);
    }
#ifdef CJVM_DEBUG_SHOW_CONSTANT_POOL_TABLE
    Inspector::printConstantPool(*this);
#endif

    // 访问限制
    raw.accessFlags = reader.readU2();
#ifdef CJVM_DEBUG_SHOW_CLASS_ACCESS_FLAGS
    Inspector::printClassAccessFlag(*this);
#endif

    // 本类及父类
    raw.thisClass = reader.readU2();
    raw.superClass = reader.readU2();

    // 接口数
    raw.interfacesCount = reader.readU2();
    if (raw.interfacesCount > 0 && !parseInterface(raw.interfacesCount)) {
        std::cerr << __func__ << ":Failed to parse interfaces\n";
        exit(EXIT_FAILURE);
    }
#ifdef CJVM_DEBUG_SHOW_INTERFACE
    Inspector::printInterfaces(*this);
#endif

    // 字段数
    raw.fieldsCount = reader.readU2();
    if (raw.fieldsCount > 0 && !parseField(raw.fieldsCount)) {
        std::cerr << __func__ << ":Failed to parse fields\n";
        exit(EXIT_FAILURE);
    }
#ifdef CJVM_DEBUG_SHOW_CLASS_FIELD
    Inspector::printField(*this);
#endif


    // 方法数
    raw.methodsCount = reader.readU2();
    if (raw.methodsCount > 0 && !parseMethod(raw.methodsCount)) {
        std::cerr << __func__ << ":Failed to parse methods\n";
        exit(EXIT_FAILURE);
    }
#ifdef CJVM_DEBUG_SHOW_CLASS_METHOD
    Inspector::printMethod(*this);
#endif

    // 属性数
    raw.attributesCount = reader.readU2();
    if (raw.attributesCount > 0 && !parseAttribute(raw.attributes, raw.attributesCount)) {
        std::cerr << __func__ << ":Failed to parse class file's attributes\n";
        exit(EXIT_FAILURE);
    }
#ifdef CJVM_DEBUG_SHOW_CLASS_ATTRIBUTE
    Inspector::printClassFileAttrs(*this);
#endif

    if (!reader.hasNoExtraBytes()) {
        std::cerr << __func__ << ":Extra bytes existed in class file\n";
        exit(EXIT_FAILURE);
    }
    return;
}


bool JavaClass::parseConstantPool(u2 cpCount) {
    raw.constPoolInfo = new ConstantPoolInfo*[cpCount];
    raw.constPoolInfo[0] = nullptr;

    if (!raw.constPoolInfo) {
        std::cerr << "Can not allocate memory to load class file\n";
        return false;
    }
}






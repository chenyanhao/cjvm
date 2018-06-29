//
// Created by ha on 18/6/16.
//

#ifndef CJVM_JAVACLASS_H
#define CJVM_JAVACLASS_H

#include <map>
#include "Type.h"
#include "JavaType.h"
#include "ClassFile.h"
#include "FileReader.h"
#include "MethodArea.h"


#define JAVA_9_MAJOR 53
#define JAVA_8_MAJOR 52
#define JAVA_7_MAJOR 51
#define JAVA_6_MAJOR 50

#define JAVA_CLASS_FILE_MAGIC_NUMBER 0XCAFEBABE

class JavaClass {
    friend struct Inspector;
    friend struct YVM;
    friend class JavaHeap;
    friend class MethodArea;
    friend class CodeExecution;
    friend class ConcurrentGC;

public:
    explicit JavaClass(const char* classFilePath);
    ~JavaClass();
    JavaClass(const JavaClass &rhs) { this->raw = rhs.raw; }

public:
    const char* getString(u2 index) const {
        return reinterpret_cast<const char*>(dynamic_cast<CONSTANT_Utf8*>(raw.constPoolInfo[index])->bytes);
    }

    const char* getClassName() const {
        return getString(dynamic_cast<CONSTANT_Class*>(raw.constPoolInfo[raw.thisClass])->nameIndex);
    }

    const char* getSuperClassName() const {
        return raw.superClass == 0
               ? nullptr
               : getString(dynamic_cast<CONSTANT_Class*>(raw.constPoolInfo[raw.superClass])->nameIndex);
    }

    bool hasSuperClass() const {
        return raw.superClass != 0;
    }

    void parseClassFile();

    std::vector<u2> getInterfacesIndex() const;

    MethodInfo* getMethod(const char *methodName, const char *methodDescriptor) const;

private:
    bool parseConstantPool(u2 cpCount);
    bool parseInterface(u2 interfaceCount);
    bool parseField(u2 fieldCount);
    bool parseMethod(u2 methodCount);
    bool parseAttribute(AttributeInfo** attrs, u2 attributeCount);

private:
    VerificationTypeInfo* determineVerificationType(u1 tag);
    TargetInfo* determineTargetType(u1 tag);
    ElementValue* readToElementValueStructure();
    Annotation readToAnnotationStructure();

private:
    ClassFile raw{};
    FileReader reader;
    std::map<size_t, JType*> sfield;
};



#endif //CJVM_JAVACLASS_H

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

// 魔数、版本号、常量池、访问限制、this/super/interface、字段、方法、属性表
void JavaClass::parseClassFile() {
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

    // JVM8 规范说明：常量池中常量的索引从 1 开始，到 constant_pool_count - 1
    for (int i = 1; i < cpCount - 1; ++i) {
        // 常量池的常量都是一个 tag 和一个表数据结构组成
        u1 tag = reader.readU1();
        ConstantPoolInfo *slot;

        switch (tag) {
            case TAG_Class:
                slot = new CONSTANT_Class();
                dynamic_cast<CONSTANT_Class*>(slot)->nameIndex = reader.readU2();
                raw.constPoolInfo[i] = dynamic_cast<CONSTANT_Class*>(slot);
                break;
            case TAG_FieldRef:
                slot = new CONSTANT_FieldRef();
                dynamic_cast<CONSTANT_FieldRef*>(slot)->classIndex = reader.readU2();
                dynamic_cast<CONSTANT_FieldRef*>(slot)->nameAndTypeIndex = reader.readU2();
                raw.constPoolInfo[i] = dynamic_cast<CONSTANT_FieldRef*>(slot);
                break;
            case TAG_MethodRef:
                slot = new CONSTANT_MethodRef();
                dynamic_cast<CONSTANT_MethodRef*>(slot)->classIndex = reader.readU2();
                dynamic_cast<CONSTANT_MethodRef*>(slot)->nameAndTypeIndex = reader.readU2();
                raw.constPoolInfo[i] = dynamic_cast<CONSTANT_MethodRef*>(slot);
                break;
            case TAG_InterfaceMethodRef:
                slot = new CONSTANT_InterfaceMethodRef();
                dynamic_cast<CONSTANT_InterfaceMethodRef*>(slot)->classIndex = reader.readU2();
                dynamic_cast<CONSTANT_InterfaceMethodRef*>(slot)->nameAndTypeIndex = reader.readU2();
                raw.constPoolInfo[i] = dynamic_cast<CONSTANT_InterfaceMethodRef*>(slot);
                break;
            case TAG_String: {
                slot = new CONSTANT_String();
                dynamic_cast<CONSTANT_String*>(slot)->stringIndex = reader.readU2();
                raw.constPoolInfo[i] = dynamic_cast<CONSTANT_String*>(slot);
                break;
            }
            case TAG_Integer: {
                slot = new CONSTANT_Integer();
                dynamic_cast<CONSTANT_Integer*>(slot)->bytes = reader.readU4();
                dynamic_cast<CONSTANT_Integer*>(slot)->val = dynamic_cast<CONSTANT_Integer*>(slot)->bytes;

                raw.constPoolInfo[i] = dynamic_cast<CONSTANT_Integer*>(slot);
                break;
            }
            case TAG_Float: {
                slot = new CONSTANT_Float();
                dynamic_cast<CONSTANT_Float*>(slot)->bytes = reader.readU4();
                dynamic_cast<CONSTANT_Float*>(slot)->val = *(float*)(&dynamic_cast<CONSTANT_Float*>(slot)->bytes);
                raw.constPoolInfo[i] = dynamic_cast<CONSTANT_Float*>(slot);
                break;
            }
            case TAG_Long: {
                slot = new CONSTANT_Long();
                dynamic_cast<CONSTANT_Long*>(slot)->highBytes = reader.readU2();
                dynamic_cast<CONSTANT_Long*>(slot)->lowBytes = reader.readU4();

                dynamic_cast<CONSTANT_Long*>(slot)->val = (((int64_t)dynamic_cast<CONSTANT_Long*>(slot)->highBytes) << 32)
                                                           + dynamic_cast<CONSTANT_Long*>(slot)->lowBytes;
                raw.constPoolInfo[i++] = dynamic_cast<CONSTANT_Long*>(slot);
                // All 8-byte constants take up two slot in the constant_pool table
                raw.constPoolInfo[i] = nullptr;
                break;
            }
            case TAG_Double: {
                slot = new CONSTANT_Double();
                dynamic_cast<CONSTANT_Double*>(slot)->highBytes = reader.readU4();
                dynamic_cast<CONSTANT_Double*>(slot)->lowBytes = reader.readU4();

                int64_t val = (((int64_t)dynamic_cast<CONSTANT_Double*>(slot)->highBytes) << 32)
                              + dynamic_cast<CONSTANT_Double*>(slot)->lowBytes;
                dynamic_cast<CONSTANT_Double*>(slot)->val = *(double*)&val;
                raw.constPoolInfo[i++] = dynamic_cast<CONSTANT_Double*>(slot);
                // All 8-byte constants take up two slot in the constant_pool table
                raw.constPoolInfo[i] = nullptr;
                break;
            }
            case TAG_NameAndType: {
                slot = new CONSTANT_NameAndType();
                dynamic_cast<CONSTANT_NameAndType*>(slot)->nameIndex = reader.readU2();
                dynamic_cast<CONSTANT_NameAndType*>(slot)->descreptorIndex = reader.readU2();
                raw.constPoolInfo[i] = dynamic_cast<CONSTANT_NameAndType*>(slot);
                break;
            }
            case TAG_Utf8: {
                slot = new CONSTANT_Utf8();
                u2 len = reader.readU2();
                dynamic_cast<CONSTANT_Utf8*>(slot)->length = len;
                dynamic_cast<CONSTANT_Utf8*>(slot)->bytes = static_cast<u1[]>(new uint8_t[len + 1]);
                //The utf8 string is not end with '\0' since we do not need to reserve extra 1 byte
                for (int k = 0; k < len; k++) {
                    dynamic_cast<CONSTANT_Utf8*>(slot)->bytes[k] = reader.readU1();
                }
                dynamic_cast<CONSTANT_Utf8*>(slot)->bytes[len] = '\0'; //End with '\0' for simplicity

                raw.constPoolInfo[i] = dynamic_cast<CONSTANT_Utf8*>(slot);
                // Todo: support unicode string ; here we just add null-char at the end of char array
                break;
            }
            case TAG_MethodHandle: {
                slot = new CONSTANT_MethodHandle();
                dynamic_cast<CONSTANT_MethodHandle*>(slot)->referenceKind = reader.readU1();
                dynamic_cast<CONSTANT_MethodHandle*>(slot)->referenceIndex = reader.readU1();
                raw.constPoolInfo[i] = dynamic_cast<CONSTANT_MethodHandle*>(slot);
                break;
            }
            case TAG_MethodType: {
                slot = new CONSTANT_MethodType();
                dynamic_cast<CONSTANT_MethodType *>(slot)->descriptorIndex = reader.readU2();
                raw.constPoolInfo[i] = dynamic_cast<CONSTANT_MethodType *>(slot);
                break;
            }
            case TAG_InvokeDynamic: {
                slot = new CONSTANT_InvokeDynamic();
                dynamic_cast<CONSTANT_InvokeDynamic *>(slot)->bootstrapMethodAttrIndex = reader.readU2();
                dynamic_cast<CONSTANT_InvokeDynamic *>(slot)->nameAndTypeIndex = reader.readU2();
                raw.constPoolInfo[i] = dynamic_cast<CONSTANT_InvokeDynamic *>(slot);
                break;
            }
            default:
                std::cerr << "undefined constant pool type\n";
                return false;
        }
    }
    return true;
}


bool JavaClass::parseInterface(u2 interfaceCount) {
    raw.interfaces = new uint16_t[interfaceCount];
    FOR_EACH(i, interfaceCount) {
        raw.interfaces[i] = reader.readU2();
        // Each index must be a valid constant pool subscript, which pointed to a CONSTANT_Class structure
        assert(typeid(*raw.constPoolInfo[raw.interfaces[i]]) == typeid(CONSTANT_Class));
    }
    return true;
}

bool JavaClass::parseField(u2 fieldCount) {
    raw.fields = new FieldInfo[fieldCount];
    if (!raw.fields) {
        std::cerr << __func__ << ":Can not allocate memory to load class file\n";
        return false;
    }

    FOR_EACH(i, fieldCount) {
        raw.fields[i].accessFlags = reader.readU2();
        raw.fields[i].nameIndex = reader.readU2();
        raw.fields[i].descriptorIndex = reader.readU2();
        raw.fields[i].attributeCount = reader.readU2();
        parseAttribute(raw.fields[i].attributes, raw.fields[i].attributeCount);
    }

    return true;
}

bool JavaClass::parseMethod(u2 methodCount) {
    raw.methods = new MethodInfo[methodCount];
    if (!raw.methods) {
        std::cerr << __func__ << ":Can not allocate memory to load class file\n";
        return false;
    }

    FOR_EACH(i, methodCount) {
        raw.methods[i].accessFlags = reader.readU2();
        raw.methods[i].nameIndex = reader.readU2();
        raw.methods[i].descriptorIndex = reader.readU2();
        raw.methods[i].attributeCount = reader.readU2();
        parseAttribute(raw.methods[i].attributes, raw.methods[i].attributeCount);
    }

    return true;
}


bool JavaClass::parseAttribute(AttributeInfo **attrs, u2 attributeCount) {
    attrs = new AttributeInfo*[attributeCount];
    if (!attrs) {
        std::cerr << __func__ << ":Can not allocate memory to load class file\n";
        return false;
    }

    FOR_EACH(i, attributeCount) {
        const u2 attrStrIndex = reader.readU2();
        if (typeid(*raw.constPoolInfo[attrStrIndex]) == typeid(CONSTANT_Utf8)) {
            return false;
        }

        char *attrName = (char*) dynamic_cast<CONSTANT_Utf8*>(raw.constPoolInfo[attrStrIndex])->bytes;
        IS_ATTR_ConstantValue(attrName) {
            auto *attr = new ATTR_ConstantValue;
            attr->attributeNameIndex = attrStrIndex;
            attr->attributeLength = reader.readU4();
            attr->constantValueIndex = reader.readU2();

            attrs[i] = attr;
            continue;
        }
        IS_ATTR_Code(attrName) {
            auto *attr = new ATTR_Code;
            attr->attributeNameIndex = attrStrIndex;
            attr->attributeLength = reader.readU4();
            attr->maxStack = reader.readU2();
            attr->maxLocals = reader.readU2();
            attr->codeLength = reader.readU4();

            attr->code = new uint8_t[attr->codeLength];
            FOR_EACH(k, attr->codeLength) {
                attr->code[k] = reader.readU1();
            }

            attr->exceptionTableLength = reader.readU2();
            attr->exceptionTable = new ATTR_Code::_ExceptionTable[attr->exceptionTableLength];
            FOR_EACH(k, attr->exceptionTableLength) {
                attr->exceptionTable[k].startPC = reader.readU2();
                attr->exceptionTable[k].endPC = reader.readU2();
                attr->exceptionTable[k].handlerPC = reader.readU2();
                attr->exceptionTable[k].catchType = reader.readU2();
            }

            attr->attributeCount = reader.readU2();
            parseAttribute(attr->attributes, attr->attributeCount);

            attrs[i] = attr;
            continue;
        }
    }
}




















































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

/**
 * 魔数、版本号、常量池、访问限制、this/super/interface、字段、方法、属性表
 *
 * @code
 * ClassFile {
 *      u4              magic
 *      u2              minor_version
 *      u2              major_version
 *      u2              constant_pool_count
 *      cp_info         constant_pool[constant_pool_count - 1]
 *      u2              access_flags
 *      u2              this_class
 *      u2              super_class
 *      u2              interfaces_count
 *      u2[]            interfaces[interfaces_count]
 *      u2              fields_count
 *      field_info      fields[fields_count]
 *      u2              methods_count
 *      method_info     methods[methods_count]
 *      u2              attributes_count
 *      attribute_info  attributes[attributes_count]
 * }
 * @endcode
 *
 */
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
                dynamic_cast<CONSTANT_NameAndType*>(slot)->descriptorIndex = reader.readU2();
                raw.constPoolInfo[i] = dynamic_cast<CONSTANT_NameAndType*>(slot);
                break;
            }
            case TAG_Utf8: {
                slot = new CONSTANT_Utf8();
                u2 len = reader.readU2();
                dynamic_cast<CONSTANT_Utf8*>(slot)->length = len;
                dynamic_cast<CONSTANT_Utf8*>(slot)->bytes = static_cast<u1*>(new uint8_t[len + 1]);
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

/**
 * @code
 * field_info {
 *      u2                  access_flag
 *      u2                  name_index
 *      u2                  descriptor_index
 *      u2                  attributes_count
 *      attribute_info      attributes[attributes_count]
 * }
 * @endcode
 *
 * @param fieldCount
 * @return
 */
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

/**
 * @code
 * method_info {
 *      u2                  access_flag
 *      u2                  name_index
 *      u2                  descriptor_index
 *      u2                  attributes_count
 *      attribute_info      attributes[attributes_count]
 * }
 * @endcode
 * @param methodCount
 * @return
 */
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


/**
 * @code
 * attribute_info {
 *      u2      attribute_name_index
 *      u4      attribute_name
 *      u1[]    info[attribute_length]
 * }
 * @endcode
 *
 * @param attrs
 * @param attributeCount
 * @return
 */
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
        IS_ATTR_StackMapTable(attrName) {
            auto *attr = new ATTR_StackMapTable;
            attr->attributeNameIndex = attrStrIndex;
            attr->attributeLength = reader.readU4();
            attr->numberOfEntries = reader.readU2();
            attr->entries = new StackMapFrame*[attr->numberOfEntries];
            FOR_EACH(k, attr->numberOfEntries) {
                u1 frameType = reader.readU1();
                if (IS_STACKFRAME_same_frame(frameType)) {
                    auto *frame = new Frame_Same();
                    attr->entries[k] = frame;
                } else if (IS_STACKFRAME_same_locals_1_stack_item_frame(frameType)) {
                    auto *frame = new Frame_Same_locals_1_stack_item;
                    frame->stack = new VerificationTypeInfo*[1];
                    frame->stack[0] = determineVerificationType(reader.readU1());
                    attr->entries[k] = frame;
                } else if (IS_STACKFRAME_same_locals_1_stack_item_frame_extended(frameType)) {
                    auto *frame = new Frame_Same_locals_1_stack_item_extended;
                    frame->offsetDelta = reader.readU2();
                    frame->stack = new VerificationTypeInfo*[1];
                    frame->stack[0] = determineVerificationType(reader.readU1());
                    attr->entries[k] = frame;
                } else if (IS_STACKFRAME_chop_frame(frameType)) {
                    auto* frame = new Frame_Chop;
                    frame->offsetDelta = reader.readU2();
                    attr->entries[k] = frame;
                } else if (IS_STACKFRAME_same_frame_extended(frameType)) {
                    auto* frame = new Frame_Same_frame_extended;
                    frame->offsetDelta = reader.readU2();
                    attr->entries[k] = frame;
                } else if (IS_STACKFRAME_append_frame(frameType)) {
                    auto* frame = new Frame_Append;
                    frame->frameType = frameType;
                    // It's important to store current frame type since ~Frame_Append need it to release memory
                    frame->offsetDelta = reader.readU2();
                    frame->stack = new VerificationTypeInfo*[frameType - 251];
                    FOR_EACH(p, frameType - 251) {
                        frame->stack[p] = determineVerificationType(reader.readU1());
                    }
                    attr->entries[k] = frame;
                } else if (IS_STACKFRAME_full_frame(frameType)) {
                    auto* frame = new Frame_Full;
                    frame->offsetDelta = reader.readU2();
                    frame->numberOfLocals = reader.readU2();
                    frame->locals = new VerificationTypeInfo*[frame->numberOfLocals];
                    FOR_EACH(p, frame->numberOfLocals) {
                        frame->locals[p] = determineVerificationType(reader.readU1());
                    }
                    frame->numberOfStackItems = reader.readU2();
                    frame->stack = new VerificationTypeInfo*[frame->numberOfStackItems];
                    FOR_EACH(p, frame->numberOfStackItems) {
                        frame->stack[p] = determineVerificationType(reader.readU1());
                    }
                    attr->entries[k] = frame;
                } else {
                    // TODO
                    // shouldn't reach here
                }
            }
            attrs[i] = attr;
            continue;
        }
        IS_ATTR_Exceptions(attrName) {
            auto *attr = new ATTR_Exception;
            attr->attributeNameIndex = attrStrIndex;
            attr->attributeLength = reader.readU4();
            attr->numberOfExceptions = reader.readU2();
            attr->exceptionIndexTable = new uint16_t[attr->numberOfExceptions];
            FOR_EACH(k, attr->numberOfExceptions) {
                attr->exceptionIndexTable[k] = reader.readU2();
            }
            attrs[i] = attr;
            continue;
        }

        IS_ATTR_InnerClasses(attrName) {
            auto *attr = new ATTR_InnerClasses;
            attr->attributeNameIndex = attrStrIndex;
            attr->attributeLength = reader.readU4();
            attr->numberOfClasses = reader.readU2();
            attr->classes = new ATTR_InnerClasses::_Classes[attr->numberOfClasses];
            FOR_EACH(k, attr->numberOfClasses) {
                attr->classes[k].innerClassInfoIndex = reader.readU2();
                attr->classes[k].outerClassInfoIndex = reader.readU2();
                attr->classes[k].innerNameIndex = reader.readU2();
                attr->classes[k].innerClassAccessFlags = reader.readU2();
            }
            attrs[i] = attr;
            continue;
        }
        IS_ATTR_EnclosingMethod(attrName) {
            auto *attr = new ATTR_EnclosingMethod;
            attr->attributeNameIndex = attrStrIndex;
            attr->attributeLength = reader.readU4();
            attr->classIndex = reader.readU2();
            attr->methodIndex = reader.readU2();
            attrs[i] = attr;
            continue;
        }
        IS_ATTR_Synthetic(attrName) {
            auto *attr = new ATTR_Synthetic;
            attr->attributeNameIndex = attrStrIndex;
            attr->attributeLength = reader.readU4();
            attrs[i] = attr;
            continue;
        }
        IS_ATTR_Signature(attrName) {
            auto *attr = new ATTR_Signature;
            attr->attributeNameIndex = attrStrIndex;
            attr->attributeLength = reader.readU4();
            attr->signatureIndex = reader.readU2();
            attrs[i] = attr;
            continue;
        }
        IS_ATTR_SourceFile(attrName) {
            auto* attr = new ATTR_SourceFile;
            attr->attributeNameIndex = attrStrIndex;
            attr->attributeLength = reader.readU4();
            attr->sourceFileIndex = reader.readU2();
            attrs[i] = attr;
            continue;
        }
        IS_ATTR_SourceDebugExtension(attrName) {
            auto* attr = new ATTR_SourceDebugExtension;
            attr->attributeNameIndex = attrStrIndex;
            attr->attributeLength = reader.readU4();
            attr->debugExtension = new uint8_t[attr->attributeLength];
            FOR_EACH(k, attr->attributeLength) {
                attr->debugExtension[k] = reader.readU1();
            }
            attrs[i] = attr;
            continue;
        }
        IS_ATTR_LineNumberTable(attrName) {
            auto* attr = new ATTR_LineNumberTable;
            attr->attributeNameIndex = attrStrIndex;
            attr->attributeLength = reader.readU4();
            attr->lineNumberTableLength = reader.readU2();
            attr->lineNumberTable = new ATTR_LineNumberTable::_LineNumberTable[attr->lineNumberTableLength];
            FOR_EACH(k, attr->lineNumberTableLength) {
                attr->lineNumberTable[k].startPC = reader.readU2();
                attr->lineNumberTable[k].lineNumber = reader.readU2();
            }
            attrs[i] = attr;
            continue;
        }
        IS_ATTR_LocalVariableTable(attrName) {
            auto* attr = new ATTR_LocalVariableTable;
            attr->attributeNameIndex = attrStrIndex;
            attr->attributeLength = reader.readU4();
            attr->localVariableTableLength = reader.readU2();
            attr->localVariableTable = new ATTR_LocalVariableTable::_LocalVariableTable[attr->localVariableTableLength];
            FOR_EACH(k, attr->localVariableTableLength) {
                attr->localVariableTable[k].startPC = reader.readU2();
                attr->localVariableTable[k].length = reader.readU2();
                attr->localVariableTable[k].nameIndex = reader.readU2();
                attr->localVariableTable[k].descriptorIndex = reader.readU2();
                attr->localVariableTable[k].index = reader.readU2();
            }
            attrs[i] = attr;
            continue;
        }
        IS_ATTR_LocalVariableTypeTable(attrName) {
            auto* attr = new ATTR_LocalVariableTypeTable;
            attr->attributeNameIndex = attrStrIndex;
            attr->attributeLength = reader.readU4();
            attr->localVariableTypeTableLength = reader.readU2();
            attr->localVariableTypeTable = new ATTR_LocalVariableTypeTable::_LocalVariableTypeTable[attr->
                    localVariableTypeTableLength];
            FOR_EACH(k, attr->localVariableTypeTableLength) {
                attr->localVariableTypeTable[k].startPC = reader.readU2();
                attr->localVariableTypeTable[k].length = reader.readU2();
                attr->localVariableTypeTable[k].nameIndex = reader.readU2();
                attr->localVariableTypeTable[k].signatureIndex = reader.readU2();
                attr->localVariableTypeTable[k].index = reader.readU2();
            }
            attrs[i] = attr;
            continue;
        }
        IS_ATTR_Deprecated(attrName) {
            auto* attr = new ATTR_Deprecated;
            attr->attributeNameIndex = attrStrIndex;
            attr->attributeLength = reader.readU4();
            attrs[i] = attr;
            continue;
        }
        IS_ATTR_RuntimeVisibleAnnotations(attrName) {
            auto* attr = new ATTR_RuntimeVisibleAnnotations;
            attr->attributeNameIndex = attrStrIndex;
            attr->attributeLength = reader.readU4();
            attr->numAnnotations = reader.readU2();
            attr->annotations = new Annotation[attr->numAnnotations];
            FOR_EACH(k, attr->numAnnotations) {
                attr->annotations[k] = readToAnnotationStructure();
            }
            attrs[i] = attr;
            continue;
        }
        IS_ATTR_RuntimeInvisibleAnnotations(attrName) {
            auto* attr = new ATTR_RuntimeInvisibleAnnotations;
            attr->attributeNameIndex = attrStrIndex;
            attr->attributeLength = reader.readU4();
            attr->numAnnotations = reader.readU2();
            attr->annotations = new Annotation[attr->numAnnotations];
            FOR_EACH(k, attr->numAnnotations) {
                attr->annotations[k] = readToAnnotationStructure();
            }
            attrs[i] = attr;
            continue;
        }
        IS_ATTR_RuntimeVisibleParameterAnnotations(attrName) {
            auto* attr = new ATTR_RuntimeVisibleParameterAnnotations;
            attr->attributeNameIndex = attrStrIndex;
            attr->attributeLength = reader.readU4();
            attr->numParameters = reader.readU1();
            attr->parameterAnnotations = new ATTR_RuntimeVisibleParameterAnnotations::_ParameterAnnotations[attr->
                    numParameters];
            FOR_EACH(k, attr->numParameters) {
                attr->parameterAnnotations[k].numAnnotations = reader.readU2();
                attr->parameterAnnotations[k].annotations = new Annotation[attr->parameterAnnotations[k].numAnnotations
                ];
                FOR_EACH(p, attr->parameterAnnotations[k].numAnnotations) {
                    attr->parameterAnnotations[k].annotations[p] = readToAnnotationStructure();
                }
            }
            attrs[i] = attr;
            continue;
        }
        IS_ATTR_RuntimeInvisibleParameterAnnotations(attrName) {
            auto* attr = new ATTR_RuntimeInvisibleParameterAnnotations;
            attr->attributeNameIndex = attrStrIndex;
            attr->attributeLength = reader.readU4();
            attr->numParameters = reader.readU1();
            attr->parameterAnnotations = new ATTR_RuntimeInvisibleParameterAnnotations::_ParameterAnnotations[attr->
                    numParameters];
            FOR_EACH(k, attr->numParameters) {
                attr->parameterAnnotations[k].numAnnotations = reader.readU2();
                attr->parameterAnnotations[k].annotations = new Annotation[attr->parameterAnnotations[k].numAnnotations
                ];
                FOR_EACH(p, attr->parameterAnnotations[k].numAnnotations) {
                    attr->parameterAnnotations[k].annotations[p] = readToAnnotationStructure();
                }
            }
            attrs[i] = attr;
            continue;
        }
        IS_ATTR_RuntimeVisibleTypeAnnotations(attrName) {
            auto* attr = new ATTR_RuntimeVisibleTypeAnnotations;
            attr->attributeNameIndex = attrStrIndex;
            attr->attributeLength = reader.readU4();
            attr->numAnnotations = reader.readU2();
            attr->annotations = new TypeAnnotation[attr->numAnnotations];
            FOR_EACH(k, attr->numAnnotations) {
                attr->annotations[k].targetType = reader.readU1();
                attr->annotations[k].targetInfo = determineTargetType(attr->annotations[k].targetType);

                // read to target_path
                attr->annotations[k].targetPath.pathLength = reader.readU1();
                attr->annotations[k].targetPath.path = new TypeAnnotation::TypePath::_Path[attr->annotations[k]
                        .targetPath.pathLength];
                FOR_EACH(p, attr->annotations[k].targetPath.pathLength) {
                    attr->annotations[k].targetPath.path[p].typePathKind = reader.readU1();
                    attr->annotations[k].targetPath.path[p].typeArgumentIndex = reader.readU1();
                }

                attr->annotations[k].typeIndex = reader.readU2();
                attr->annotations[k].numElementValuePairs = reader.readU2();
                attr->annotations[k].elementValuePairs = new TypeAnnotation::_ElementValuePairs[attr->annotations[k].
                        numElementValuePairs];
                FOR_EACH(p, attr->annotations[k].numElementValuePairs) {
                    attr->annotations[k].elementValuePairs[p].elementNameIndex = reader.readU2();
                    attr->annotations[k].elementValuePairs[p].value = readToElementValueStructure();
                }
            }
            attrs[i] = attr;
            continue;
        }
        IS_ATTR_RuntimeInvisibleTypeAnnotations(attrName) {
            auto* attr = new ATTR_RuntimeInvisibleTypeAnnotations;
            attr->attributeNameIndex = attrStrIndex;
            attr->attributeLength = reader.readU4();
            attr->numAnnotations = reader.readU2();
            attr->annotations = new TypeAnnotation[attr->numAnnotations];
            FOR_EACH(k, attr->numAnnotations) {
                attr->annotations[k].targetType = reader.readU1();
                attr->annotations[k].targetInfo = determineTargetType(attr->annotations[k].targetType);

                // read to target_path
                attr->annotations[k].targetPath.pathLength = reader.readU1();
                attr->annotations[k].targetPath.path = new TypeAnnotation::TypePath::_Path[attr->annotations[k]
                        .targetPath.pathLength];
                FOR_EACH(p, attr->annotations[k].targetPath.pathLength) {
                    attr->annotations[k].targetPath.path[p].typePathKind = reader.readU1();
                    attr->annotations[k].targetPath.path[p].typeArgumentIndex = reader.readU1();
                }

                attr->annotations[k].typeIndex = reader.readU2();
                attr->annotations[k].numElementValuePairs = reader.readU2();
                attr->annotations[k].elementValuePairs = new TypeAnnotation::_ElementValuePairs[attr->annotations[k].
                        numElementValuePairs];
                FOR_EACH(p, attr->annotations[k].numElementValuePairs) {
                    attr->annotations[k].elementValuePairs[p].elementNameIndex = reader.readU2();
                    attr->annotations[k].elementValuePairs[p].value = readToElementValueStructure();
                }
            }
            attrs[i] = attr;
            continue;
        }
        IS_ATTR_AnnotationDefault(attrName) {
            auto* attr = new ATTR_AnnotationDefault;
            attr->attributeNameIndex = attrStrIndex;
            attr->attributeLength = reader.readU4();
            attr->defaultValue = readToElementValueStructure();
            attrs[i] = attr;
            continue;
        }
        IS_ATTR_BootstrapMethods(attrName) {
            auto* attr = new ATTR_BootstrapMethods;
            attr->attributeNameIndex = attrStrIndex;
            attr->attributeLength = reader.readU4();
            attr->numBootstrapMethods = reader.readU2();
            attr->bootstrapMethod = new ATTR_BootstrapMethods::_BootstrapMethod[attr->numBootstrapMethods];
            FOR_EACH(k, attr->numBootstrapMethods) {
                attr->bootstrapMethod[k].bootstrapMethodRef = reader.readU2();

                attr->bootstrapMethod[k].numBootstrapArgument = reader.readU2();
                attr->bootstrapMethod[k].bootstrapArguments = new uint16_t[attr->bootstrapMethod[k].numBootstrapArgument];
                FOR_EACH(p, attr->bootstrapMethod[k].numBootstrapArgument) {
                    attr->bootstrapMethod[k].bootstrapArguments[p] = reader.readU2();
                }
            }
            attrs[i] = attr;
            continue;
        }
        IS_ATTR_MethodParameters(attrName) {
            auto* attr = new ATTR_MethodParameter;
            attr->attributeNameIndex = attrStrIndex;
            attr->attributeLength = reader.readU4();
            attr->parameterCount = reader.readU1();
            attr->parameters = new ATTR_MethodParameter::_Parameters[attr->parameterCount];
            FOR_EACH(k, attr->parameterCount) {
                attr->parameters[k].nameIndex = reader.readU2();
                attr->parameters[k].accessFlags = reader.readU2();
            }
            attrs[i] = attr;
        }

    }
}


VerificationTypeInfo* JavaClass::determineVerificationType(u1 tag) {
    switch (tag) {
        case ITEM_Top:
            return new VariableInfo_Top();
        case ITEM_Integer:
            return new VariableInfo_Integer();
        case ITEM_Float:
            return new VariableInfo_Float;
        case ITEM_Null: {
            return new VariableInfo_Null;
        }
        case ITEM_UninitializedThis: {
            return new VariableInfo_UninitializedThis;
        }
        case ITEM_Object: {
            auto* x = new VariableInfo_Object;
            x->cpoolIndex = reader.readU2();
            return x;
        }
        case ITEM_Uninitialized: {
            auto* x = new VariableInfo_Uninitialized;
            x->offset = reader.readU2();
            return x;
        }
        case ITEM_Long: {
            return new VariableInfo_Long;
        }
        case ITEM_Double: {
            return new VariableInfo_Double;
        }
        default:
            std::cerr << __func__ << ":Incorrect tag of verification type\n";
            return nullptr;
    }
}


TargetInfo* JavaClass::determineTargetType(u1 tag) {
    if (tag == 0x00 || tag == 0x01) {
        auto* t = new Target_TypeParameter;
        t->typeParameterIndex = reader.readU1();
        return t;
    }
    if (tag == 0x10) {
        auto* t = new Target_SuperType;
        t->superTypeIndex = reader.readU2();
        return t;
    }
    if (tag == 0x11 || tag == 0x12) {
        auto* t = new Target_TypeParameterBound;
        t->typeParameterIndex = reader.readU1();
        t->boundIndex = reader.readU1();
        return t;
    }
    if (tag == 0x13 || tag == 0x14 || tag == 0x15) {
        return new Target_Empty;
    }
    if (tag == 0x16) {
        auto* t = new Target_FormalParameter;
        t->formalParameter = reader.readU1();
        return t;
    }
    if (tag == 0x17) {
        auto* t = new Target_Throws;
        t->throwsTypeIndex = reader.readU2();
        return t;
    }
    if (tag == 0x40 || tag == 0x41) {
        auto* t = new Target_LocalVar;
        t->tableLength = reader.readU2();
        FOR_EACH(i, t->tableLength) {
            t->table[i].startPc = reader.readU2();
            t->table[i].length = reader.readU2();
            t->table[i].index = reader.readU2();
        }
        return t;
    }
    if (tag == 0x42) {
        auto* t = new Target_Catch;
        t->exceptionTableIndex = reader.readU2();
        return t;
    }
    if (tag >= 0x43 && tag <= 0x46) {
        auto* t = new Target_Offset;
        t->offset = reader.readU2();
        return t;
    }
    if (tag >= 0x47 && tag <= 0x4B) {
        auto* t = new Target_TypeArgument;
        t->offset = reader.readU2();
        t->typeArgumentIndex = reader.readU1();
        return t;
    }
}


Annotation JavaClass::readToAnnotationStructure() {
    Annotation a{};
    a.typeIndex = reader.readU2();
    a.numElementValuePairs = reader.readU2();
    FOR_EACH(p, a.numElementValuePairs) {
        a.elementValuePairs[p].elementNameIndex = reader.readU2();
        a.elementValuePairs[p].value = readToElementValueStructure();
    }
    return a;
}


ElementValue* JavaClass::readToElementValueStructure() {
    char tag = reader.readU1();
    switch (tag) {
        case 'B':
        case 'C':
        case 'D':
        case 'F':
        case 'I':
        case 'J':
        case 'S':
        case 'Z':
        case 's': {
            // const_value_index of union
            auto* e = new ElementValue_ConstantValueIndex;
            e->constValueIndex = reader.readU2();
            return e;
        }
        case 'e': {
            auto* e = new ElementValue_EnumConstValue;
            e->typeNameIndex = reader.readU2();
            e->constNameIndex = reader.readU2();
            break;
        }
        case 'c': {
            auto* e = new ElementValue_ClassInfoIndex;
            e->classInfoIndex = reader.readU2();
            return e;
        }
        case '@': {
            auto* e = new ElementValue_Annotation;
            e->annotationValue = readToAnnotationStructure();
            return e;
        }
        case '[': {
            auto* e = new ElementValue_ArrayValue;
            e->numValues = reader.readU2();
            e->values = new ElementValue *[e->numValues];
            FOR_EACH(i, e->numValues) {
                e->values[i] = readToElementValueStructure();
            }
            return e;
        }
        default:
            std::cerr << __func__ << ":Incorrect element value type\n";
            return nullptr;
    }

}












































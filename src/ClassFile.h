//
// Created by cyh on 2018/5/31.
//

#ifndef CJVM_CLASSFILE_H
#define CJVM_CLASSFILE_H

#include "Util.h"
#include "Type.h"

/****************************************************************************
* Constant tags
****************************************************************************/

enum ConstantTag {
    TAG_Utf8 = 1,

    TAG_Integer = 3,
    TAG_Float = 4,
    TAG_Long = 5,
    TAG_Double = 6,

    TAG_Class = 7,
    TAG_String = 8,
    TAG_FieldRef = 9,
    TAG_MethodRef = 10,
    TAG_InterfaceMethodRef = 11,
    TAG_NameAndType = 12,

    TAG_MethodHandle = 15,
    TAG_MethodType = 16,
    TAG_InvokeDynamic = 18,
};


enum VariableInfoTag {
    ITEM_Top = 0,

    ITEM_Integer = 1,
    ITEM_Float = 2,
    ITEM_Double = 3,
    ITEM_Long = 4,
    ITEM_Null = 5,
    ITEM_UninitializedThis = 6,

    ITEM_Object = 7,

    ITEM_Uninitialized = 8,
};




/****************************************************************************
 * ConstantPool definitions
 ****************************************************************************/
// ====== 常量池 ======
class ConstantPoolInfo {
public:
    virtual ~ConstantPoolInfo() = default;
};


#define DEF_CONSTANT_WITH_2_FIELDS(name, type, field) \
class CONSTANT_##name : public ConstantPoolInfo { \
public: \
    static const u1 tag = ConstantTag::TAG_##name; \
    type field; \
};

#define DEF_CONSTANT_WITH_3_FIELDS(name, type1, field1, type2, field2) \
class CONSTANT_##name : public ConstantPoolInfo { \
public: \
    static const u1 tag = ConstantTag::TAG_##name; \
    type1 field1; \
    type2 field2; \
};

#define DEF_CONSTANT_WITH_4_FIELDS(name,type1,field1,type2,field2,type3,field3) \
class CONSTANT_##name : public ConstantPoolInfo{ \
public: \
    static const u1 tag = ConstantTag::TAG_##name; \
    type1 field1; \
    type2 field2; \
    type3 field3; \
};


// 下面这些和 ConstantTag 中的枚举对应
DEF_CONSTANT_WITH_2_FIELDS(String, u2, stringIndex);
DEF_CONSTANT_WITH_3_FIELDS(Integer, u4, bytes, int32_t, val);
DEF_CONSTANT_WITH_3_FIELDS(Float, u4, bytes, float, val);
DEF_CONSTANT_WITH_4_FIELDS(Long, u4, highBytes, u4, lowBytes, int64_t, val);
DEF_CONSTANT_WITH_4_FIELDS(Double, u4, highBytes, u4, lowBytes, double, val);

/**
 * nameIndex:   指向全限定名常量项的索引
 */
DEF_CONSTANT_WITH_2_FIELDS(Class, u2, nameIndex);

/**
 * classIndex:          指向声明字段的类或者接口描述符 CONSTANT_Class_info 的索引项
 * nameAndTypeIndex:    指向该字段描述符 CONSTANT_NameAndTypeinfo 的索引项
 */
DEF_CONSTANT_WITH_3_FIELDS(FieldRef, u2, classIndex, u2, nameAndTypeIndex);

/**
 * classIndex:          指向声明方法的类描述符 CONSTANT_Class_info 的索引项
 * nameAndTypeIndex:    指向该方法描述符 CONSTANT_NameAndTypeinfo 的索引项
 */
DEF_CONSTANT_WITH_3_FIELDS(MethodRef, u2, classIndex, u2, nameAndTypeIndex);

/**
 * classIndex:          指向声明方法的接口描述符 CONSTANT_Class_info 的索引项
 * nameAndTypeIndex:    指向该方法描述符 CONSTANT_NameAndTypeinfo 的索引项
 */
DEF_CONSTANT_WITH_3_FIELDS(InterfaceMethodRef, u2, classIndex, u2, nameAndTypeIndex);

/**
 * nameIndex:       指向该字段或方法名称常量项的索引
 * descriptorIndex: 指向该字段或方法描述符常量项的索引
 */
DEF_CONSTANT_WITH_3_FIELDS(NameAndType, u2, nameIndex, u2, descriptorIndex);

/**
 * referenceKind:   值必须在 1-9 之间，它决定了方法句柄的类型。方法举兵类型的值表示方法句柄的字节码行为
 * referenceIndex:  值必须是对常量池的有效索引
 */
DEF_CONSTANT_WITH_3_FIELDS(MethodHandle, u1, referenceKind, u2, referenceIndex);

/**
 * descriptorIndex: 对常量池的索引，常量池在该索引处的项必须是 CONSTANT_Utf8_info 结构，表示该方法的描述符
 */
DEF_CONSTANT_WITH_2_FIELDS(MethodType, u2, descriptorIndex);

/**
 * bootstrapMethodAttrIndex:    当前 Class 文件中引导方法表的 bootstrap_methods[] 数组的索引
 * nameAndTypeIndex:            对常量池的索引，常量池在该索引处的项必须是 CONSTANT_NameAndTypeinfo 结构，表示方法名和方法的描述符
 */
DEF_CONSTANT_WITH_3_FIELDS(InvokeDynamic, u2, bootstrapMethodAttrIndex, u2, nameAndTypeIndex);

/**
 * length:  Utf-8 编码的字符串占用的字节数
 * bytes:   长度为 length 的 Utf-8 字符串
 *
 * 其属性有动态数组，所以需要 override 其析构函数，
 * 所以这里没有用宏来定义。
 */
class CONSTANT_Utf8 : public ConstantPoolInfo {
public:
    static const u1 tag = ConstantTag::TAG_Utf8;
    u2 length;
    u1 *bytes;

    ~CONSTANT_Utf8() override {
        delete[] bytes;
    }
};




/****************************************************************************
 * Attributes definitions
 ****************************************************************************/
// ====== 属性表。class 文件最复杂、最具扩展的表 ======
class AttributeInfo {
public:
    u2 attributeNameIndex;
    u4 attributeLength;
    virtual ~AttributeInfo() = default;
};

#define DEF_ATTR_START(name) \
class ATTR_##name : public AttributeInfo

/**
 * final 关键字定义的常量值，该属性的作用是通知虚拟机自动为静态变量赋值
 */
DEF_ATTR_START(ConstantValue) {
public:
    u2 constantValueIndex;
};

/**
 * Java 代码编译成的字节码指令
 * maxStack:    操作数栈(Operand Stacks)的最大深度
 * maxLocals:   局部变量表存储空间
 * codeLength:  编译后的字节码指令的长度
 * code:        编译后的字节码指令
 *
 */
DEF_ATTR_START(Code) {
public:
    u2 maxStack;
    u2 maxLocals;

    u4 codeLength;
    u1 *code;

    u2 exceptionTableLength;
    class _ExceptionTable {
    public:
        u2 startPC;
        u2 endPC;
        u2 handlerPC;
        u2 catchType;
    } *exceptionTable;

    u2 attributeCount;
    AttributeInfo **attributes;

    ~ATTR_Code() override {
        delete [] code;
        delete [] exceptionTable;

        FOR_EACH(i, attributeCount) {
            delete attributes[i];
        }
        delete  attributes;
    }

};


class VerificationTypeInfo {
public:
    virtual ~VerificationTypeInfo() = default;
};

#define DEF_VARIABLE_INFO_WITH_1_FIELDS(name) \
class VariableInfo_##name : public VerificationTypeInfo { \
public: \
    static const u1 tag = VariableInfoTag::ITEM_##name; \
};

#define DEF_VARIABLE_INFO_WITH_2_FIELDS(name, type, field) \
class VariableInfo_##name : public VerificationTypeInfo { \
public: \
    static const u1 tag = VariableInfoTag::ITEM_##name; \
    type field; \
};

DEF_VARIABLE_INFO_WITH_1_FIELDS(Top);
DEF_VARIABLE_INFO_WITH_1_FIELDS(Integer);
DEF_VARIABLE_INFO_WITH_1_FIELDS(Float);
DEF_VARIABLE_INFO_WITH_1_FIELDS(Null);
DEF_VARIABLE_INFO_WITH_1_FIELDS(UninitializedThis);
DEF_VARIABLE_INFO_WITH_2_FIELDS(Object,u2,cpoolIndex);
DEF_VARIABLE_INFO_WITH_2_FIELDS(Uninitialized,u2,offset);
DEF_VARIABLE_INFO_WITH_1_FIELDS(Long);
DEF_VARIABLE_INFO_WITH_1_FIELDS(Double);


class StackMapFrame {
public:
    virtual ~StackMapFrame() = default;
};

#define DEF_FRAME_TYPE_WITH_1_FIELDS(name)  \
class Frame_##name : public StackMapFrame{    \
public: \
    u2 frameType;   \
};
#define DEF_FRAME_TYPE_WITH_2_FIELDS(name,type,field)  \
class Frame_##name : public StackMapFrame{\
public: \
    u2 frameType;   \
    type field;     \
};
#define DEF_FRAME_TYPE_WITH_3_FIELDS(name,type,field,type1,field1)  \
class Frame_##name : public StackMapFrame{\
    u2 frameType;   \
    type field;     \
    type1 field1;   \
};

DEF_FRAME_TYPE_WITH_1_FIELDS(Same);
DEF_FRAME_TYPE_WITH_2_FIELDS(Chop,u2,offsetDelta);
DEF_FRAME_TYPE_WITH_2_FIELDS(Same_frame_extended,u2,offsetDelta);

class Frame_Same_locals_1_stack_item : public StackMapFrame {
public:
    u2 frameType;
    VerificationTypeInfo **stack;

    ~Frame_Same_locals_1_stack_item() override {
        delete stack[0];
        delete stack;
    }
};

class Frame_Same_locals_1_stack_item_extended : public StackMapFrame {
public:
    u2 frameType;
    u2 offsetDelta;
    VerificationTypeInfo **stack;

    ~Frame_Same_locals_1_stack_item_extended() override {
        delete stack[0];
        delete stack;
    }
};

class Frame_Append : public StackMapFrame {
public:
    u2 frameType;
    u2 offsetDelta;
    VerificationTypeInfo **stack;

    ~Frame_Append() override {
        FOR_EACH(i,frameType-251) {
            delete stack[i];
        }
        delete[] stack;
    }
};

class Frame_Full : public StackMapFrame {
public:
    u2 frameType;
    u2 offsetDelta;

    u2 numberOfLocals;
    VerificationTypeInfo **locals;

    u2 numberOfStackItems;
    VerificationTypeInfo **stack;

    ~Frame_Full() override {
        FOR_EACH(i,numberOfLocals) {
            delete locals[i];
        }
        delete[] locals;
        FOR_EACH(i,numberOfStackItems) {
            delete stack[i];
        }
        delete[] stack;
    }
};

/**
 * Code 属性中使用，JDK1.6 中新增的属性
 *
 * 供新的类型检查验证器检查和处理目标方法的局部变量和操作数栈所需的类型是否匹配，
 * 目的在于代替一千比较消耗性能的基于数据流分析的类型推导验证器。
 *
 * 其中包含零至多个栈映射帧(StackMapFrame)，每个 StackMapFrame 都显式或隐式地代表了一个字节码偏移量，
 * 用于表示执行到该字节码时局部变量表和操作数栈的验证类型。
 */
DEF_ATTR_START(StackMapTable) {
public:
    u2 numberOfEntries;
    StackMapFrame **entries;

    ~ATTR_StackMapTable() override {
        FOR_EACH(i,numberOfEntries) {
            delete entries[i];
        }
        delete[] entries;
    }
};

// 异常表
/**
 * Exception 属性不要与 Code 属性中用到的异常表混淆
 *
 * Exception 属性的作用是列举方法中可能抛出的受检异常，
 * 每一种受检异常用一个 exceptionIndexTable 表示，它是一个指向常量池中 CONSTANT_Class_info 型常量的指针
 */
DEF_ATTR_START(Exception) {
public:
    u2 numberOfExceptions;
    u2 *exceptionIndexTable;

    ~ATTR_Exception() override {
        delete[] exceptionIndexTable;
    }
};

// 内部类
/**
 * 记录内部类与宿主类之间的关联
 *
 * numberOfClasses:         记录多少个内部类
 *
 * innerClassInfoIndex:     指向常量池中 CONSTANT_Class_info 型常量的指针，代表内部类的符号引用
 * outerClassInfoIndex:     指向常量池中 CONSTANT_Class_info 型常量的指针，代表宿主类的符号引用
 * innerNameIndex:          指向常量池中 CONSTANT_Utf8_info 型常量的索引，代表这个内部类的名称，匿名内部类这项值为 0
 *
 */
DEF_ATTR_START(InnerClasses) {
public:
    u2 numberOfClasses;
    class _Classes {
    public:
        u2 innerClassInfoIndex;
        u2 outerClassInfoIndex;
        u2 innerNameIndex;
        u2 innerClassAccessFlags;
    } *classes;

    ~ATTR_InnerClasses() override {
        delete[] classes;
    }
};

DEF_ATTR_START(EnclosingMethod) {
public:
    u2 classIndex;
    u2 methodIndex;
};

/**
 * 表示此字段或方法并不是由 Java 源码直接产生的，而是由编译器自行添加的
 */
DEF_ATTR_START(Synthetic) {};

/**
 * JDK1.5 发布后增加的属性，是一个可选的定长属性，
 * 可以出现在类、字段表、方法表结构的属性表中。
 *
 * signatureIndex:      指向常量池中 CONSTANT_Utf8_info 型常量的索引，代表类签名、方法类型签名、字段类型签名
 *
 * 因为 Java 的伪泛型，所以用这样一个属性去记录泛型类型，弥补一些伪泛型的缺陷。
 * （现在 Java 的反射 API 能够获取泛型类型，最终的数据来源也就是这个属性。）
 */
DEF_ATTR_START(Signature) {
public:
    u2 signatureIndex;
};

/**
 * 记录生成这个 Class 文件的源码文件的名称
 */
DEF_ATTR_START(SourceFile) {
public:
    u2 sourceFileIndex;
};
DEF_ATTR_START(SourceDebugExtension) {
public:
    u1 *debugExtension;
    ~ATTR_SourceDebugExtension() override {
        delete[] debugExtension;
    }
};

/**
 * 描述 Java 源码与字节码行号之间的对应关系，它不是运行时必需的属性
 */
DEF_ATTR_START(LineNumberTable) {
public:
    u2 lineNumberTableLength;
    class _LineNumberTable {
    public:
        u2 startPC;
        u2 lineNumber;
    } *lineNumberTable;

    ~ATTR_LineNumberTable() override {
        delete[] lineNumberTable;
    }
};

/**
 * 描述栈帧中局部变量表中的变量与 Java 源码中定义的变量之间的关系，它不是运行时必需的属性
 */
DEF_ATTR_START(LocalVariableTable) {
public:
    u2 localVariableTableLength;
    class _LocalVariableTable {
    public:
        u2 startPC;
        u2 length;
        u2 nameIndex;
        u2 descriptorIndex;
        u2 index;
    } *localVariableTable;

    ~ATTR_LocalVariableTable() override {
        delete[] localVariableTable;
    }
};

DEF_ATTR_START(LocalVariableTypeTable) {
public:
    u2 localVariableTypeTableLength;
    class _LocalVariableTypeTable {
    public:
        u2 startPC;
        u2 length;
        u2 nameIndex;
        u2 signatureIndex;
        u2 index;
    } *localVariableTypeTable;

    ~ATTR_LocalVariableTypeTable() override {
        delete[] localVariableTypeTable;
    }
};

DEF_ATTR_START(Deprecated) {};



class ElementValue {
public:
    u1 tag;
    virtual ~ElementValue() = default;
};

class Annotation {
public:
    u2 typeIndex;

    u2 numElementValuePairs;
    class _ElementValuePairs {
    public:
        u2 elementNameIndex;
        ElementValue* value;
        ~_ElementValuePairs() {
            delete value;
        }
    } *elementValuePairs;

    ~Annotation() {
        delete[] elementValuePairs;
    }
};


class ElementValue_ConstantValueIndex : public ElementValue {
public:
    u2 constValueIndex;
};

class ElementValue_EnumConstValue : public ElementValue {
public:
    u2 typeNameIndex;
    u2 constNameIndex;
};

class ElementValue_ClassInfoIndex : public ElementValue {
public:
    u2 classInfoIndex;
};

class ElementValue_ArrayValue : public ElementValue {
public:
    u2 numValues;
    ElementValue **values;

    ~ElementValue_ArrayValue() override {
        FOR_EACH(i, numValues) {
            delete values[i];
        }
        delete [] values;
    }
};

class ElementValue_Annotation : public ElementValue {
public:
    Annotation annotationValue;
};

DEF_ATTR_START(RuntimeVisibleAnnotations) {
public:
    u2 numAnnotations;
    Annotation *annotations;

    ~ATTR_RuntimeVisibleAnnotations() override {
        delete[] annotations;
    }
};

DEF_ATTR_START(RuntimeInvisibleAnnotations) {
public:
    u2 numAnnotations;
    Annotation *annotations;

    ~ATTR_RuntimeInvisibleAnnotations() override {
        delete[] annotations;
    }
};

DEF_ATTR_START(RuntimeVisibleParameterAnnotations) {
public:
    u1 numParameters;

    class _ParameterAnnotations {
    public:
        u2 numAnnotations;
        Annotation* annotations;

        ~_ParameterAnnotations() {
            delete[] annotations;
        }
    } *parameterAnnotations;

    ~ATTR_RuntimeVisibleParameterAnnotations() override {
        delete[] parameterAnnotations;
    }
};

DEF_ATTR_START(RuntimeInvisibleParameterAnnotations) {
public:
    u1 numParameters;

    class _ParameterAnnotations {
    public:
        u2 numAnnotations;
        Annotation *annotations;

        ~_ParameterAnnotations() {
            delete[] annotations;
        }
    } *parameterAnnotations;

    ~ATTR_RuntimeInvisibleParameterAnnotations() override {
        delete[] parameterAnnotations;
    }
};


class TargetInfo {
public:
    virtual ~TargetInfo() = default;
};

#define DEF_TARGET_WITH_1_FIELDS(name,type,field)  \
class Target_##name : public TargetInfo{   \
public: \
    type field; \
};
#define DEF_TARGET_WITH_2_FIELDS(name,type,field,type1,field1)  \
class Target_##name : public TargetInfo{   \
public: \
    type field; \
    type1 field1;   \
};

DEF_TARGET_WITH_1_FIELDS(TypeParameter,u1,typeParameterIndex);
DEF_TARGET_WITH_1_FIELDS(SuperType,u2,superTypeIndex);
DEF_TARGET_WITH_2_FIELDS(TypeParameterBound,u1,typeParameterIndex,u1,boundIndex);

class Target_Empty : public TargetInfo {};

DEF_TARGET_WITH_1_FIELDS(FormalParameter,u1,formalParameter);
DEF_TARGET_WITH_1_FIELDS(Throws,u2,throwsTypeIndex);

class Target_LocalVar : public TargetInfo {
public:
    u2 tableLength;

    class _Table {
    public:
        u2 startPc;
        u2 length;
        u2 index;
    } *table;

    ~Target_LocalVar() override {
        delete[] table;
    }
};

DEF_TARGET_WITH_1_FIELDS(Catch,u2,exceptionTableIndex);
DEF_TARGET_WITH_1_FIELDS(Offset,u2,offset);
DEF_TARGET_WITH_2_FIELDS(TypeArgument,u2,offset,u1,typeArgumentIndex);

class TypeAnnotation {
public:
    u1 targetType;
    TargetInfo *targetInfo;

    class TypePath {
    public:
        u1 pathLength;

        class _Path {
        public:
            u1 typePathKind;
            u1 typeArgumentIndex;
        } *path;

        ~TypePath() {
            delete [] path;
        }
    } targetPath;

    u2 typeIndex;
    u2 numElementValuePairs;

    class _ElementValuePairs {
    public:
        u2 elementNameIndex;
        ElementValue *value;

        ~_ElementValuePairs() {
            delete value;
        }
    } *elementValuePairs;

    ~TypeAnnotation() {
        delete targetInfo;
        delete [] elementValuePairs;
    }
};

DEF_ATTR_START(RuntimeVisibleTypeAnnotations) {
public:
    u2 numAnnotations;
    TypeAnnotation *annotations;

    ~ATTR_RuntimeVisibleTypeAnnotations() override {
        delete[] annotations;
    }
};

DEF_ATTR_START(RuntimeInvisibleTypeAnnotations) {
public:
    u2 numAnnotations;
    TypeAnnotation *annotations;

    ~ATTR_RuntimeInvisibleTypeAnnotations() override {
        delete[] annotations;
    }
};

DEF_ATTR_START(AnnotationDefault) {
public:
    ElementValue *defaultValue;

    ~ATTR_AnnotationDefault() {
        delete defaultValue;
    }
};

/**
 * JDK1.7 发布后新增
 *
 * numBootstrapMethods:     引导方法限定符的数量
 * bootstrapMethod:         该数组中每个成员指向一个引导方法的信息
 *
 * bootstrapMethodRef:      指向常量池中 CONSTANT_MethodHandle_info 型结构的索引
 * numBootstrapArgument:    给出了 bootstrapArguments 数组的数量
 * bootstrapArguments:      每个成员是一个对常量池的索引，常量池在该索引处必须是以下结构之一，
 *                                      CONSTANT_String_info
 *                                      CONSTANT_Class_info
 *                                      CONSTANT_Integer_info
 *                                      CONSTANT_Long_info
 *                                      CONSTANT_Float_info
 *                                      CONSTANT_Double_info
 *                                      CONSTANT_MethodHandle_info
 *                                      CONSTANT_MethodType_info
 */
DEF_ATTR_START(BootstrapMethods) {
public:
    u2 numBootstrapMethods;

    class _BootstrapMethod {
    public:
        u2 bootstrapMethodRef;
        u2 numBootstrapArgument;
        u2 *bootstrapArguments;

        ~_BootstrapMethod() {
            delete[] bootstrapArguments;
        }
    } *bootstrapMethod;

    ~ATTR_BootstrapMethods() override {
        delete[] bootstrapMethod;
    }
};

DEF_ATTR_START(MethodParameter) {
public:
    u1 parameterCount;

    class _Parameters {
    public:
        u2 nameIndex;
        u2 accessFlags;
    } *parameters;

    ~ATTR_MethodParameter() override {
        delete[] parameters;
    }
};



/****************************************************************************
 * FieldInfo definition
 ****************************************************************************/
// ====== 字段 ======
class FieldInfo {
public:
    u2 accessFlags;
    u2 nameIndex;
    u2 descriptorIndex;
    u2 attributeCount;
    AttributeInfo **attributes;

    ~FieldInfo() {
        FOR_EACH(i, attributeCount) {
            delete attributes[i];
        }
        if (attributeCount > 0) {
            delete[] attributes;
        }
    }
};


/****************************************************************************
 * MethodInfo definition
 ****************************************************************************/
// ====== 方法 ======
class MethodInfo {
public:
    u2 accessFlags;
    u2 nameIndex;
    u2 descriptorIndex;
    u2 attributeCount;
    AttributeInfo **attributes;

    ~MethodInfo() {
        FOR_EACH(i, attributeCount) {
            delete attributes[i];
        }
        delete[] attributes;
    }
};



/****************************************************************************
 * Raw java class file format
 ****************************************************************************/
class ClassFile {
public:
    // 魔数
    u4 magic;

    // 版本
    u2 minorVersion;
    u2 majorVersion;

    // 常量池
    u2 constPoolCount;
    ConstantPoolInfo **constPoolInfo;

    // 访问限制
    u2 accessFlags;

    // this/super/interfaces
    u2 thisClass;
    u2 superClass;
    u2 interfacesCount;
    u2 *interfaces;

    // 字段
    u2 fieldsCount;
    FieldInfo *fields;

    // 方法
    u2 methodsCount;
    MethodInfo *methods;

    // 属性表
    u2 attributesCount;
    AttributeInfo **attributes;

    ~ClassFile() {
        FOR_EACH(i, constPoolCount) {
            delete constPoolInfo[i];
        }
        delete[] constPoolInfo;

        if (interfacesCount > 0) {
            delete[] interfaces;
        }

        if (fieldsCount > 0) {
            delete[] fields;
        }

        if (methodsCount > 0) {
            delete[] methods;
        }

        FOR_EACH(i, attributesCount) {
            delete attributes[i];
        }
        delete[] attributes;
    }
};


/**
*\brief  Utilities for parsing class file
*/
#define IS_ATTR_ConstantValue(PTR) if(strcmp(PTR,"ConstantValue")==0)
#define IS_ATTR_Code(PTR) if(strcmp(PTR,"Code")==0)
#define IS_ATTR_StackMapTable(PTR) if(strcmp(PTR,"StackMapTable")==0)
#define IS_ATTR_Exceptions(PTR) if(strcmp(PTR,"Exceptions")==0)
#define IS_ATTR_BootstrapMethods(PTR) if(strcmp(PTR,"BootstrapMethods")==0)
#define IS_ATTR_InnerClasses(PTR) if(strcmp(PTR,"InnerClasses")==0)
#define IS_ATTR_EnclosingMethod(PTR) if(strcmp(PTR,"EnclosingMethod")==0)
#define IS_ATTR_Synthetic(PTR) if(strcmp(PTR,"Synthetic")==0)
#define IS_ATTR_Signature(PTR) if(strcmp(PTR,"Signature")==0)
#define IS_ATTR_RuntimeVisibleAnnotations(PTR) if(strcmp(PTR,"RuntimeVisibleAnnotations")==0)
#define IS_ATTR_RuntimeInvisibleAnnotations(PTR) if(strcmp(PTR,"RuntimeInvisibleAnnotations")==0)
#define IS_ATTR_RuntimeVisibleParameterAnnotations(PTR) if(strcmp(PTR,"RuntimeVisibleParameterAnnotations")==0)
#define IS_ATTR_RuntimeInvisibleParameterAnnotations(PTR) if(strcmp(PTR,"RuntimeInvisibleParameterAnnotations")==0)
#define IS_ATTR_RuntimeVisibleTypeAnnotations(PTR) if(strcmp(PTR,"RuntimeVisibleTypeAnnotations")==0)
#define IS_ATTR_RuntimeInvisibleTypeAnnotations(PTR) if(strcmp(PTR,"RuntimeInvisibleTypeAnnotations")==0)
#define IS_ATTR_AnnotationDefault(PTR) if(strcmp(PTR,"AnnotationDefault")==0)
#define IS_ATTR_MethodParameters(PTR) if(strcmp(PTR,"MethodParameters")==0)
#define IS_ATTR_SourceFile(PTR) if(strcmp(PTR,"SourceFile")==0)
#define IS_ATTR_SourceDebugExtension(PTR) if(strcmp(PTR,"SourceDebugExtension")==0)
#define IS_ATTR_LineNumberTable(PTR) if(strcmp(PTR,"LineNumberTable")==0)
#define IS_ATTR_LocalVariableTable(PTR) if(strcmp(PTR,"LocalVariableTable")==0)
#define IS_ATTR_LocalVariableTypeTable(PTR) if(strcmp(PTR,"LocalVariableTypeTable")==0)
#define IS_ATTR_Deprecated(PTR) if(strcmp(PTR,"Deprecated")==0)

#define IS_STACKFRAME_same_frame(num) ((num) >= 0&& (num) <= 63)
#define IS_STACKFRAME_same_locals_1_stack_item_frame(num) ((num) >= 64&& (num) <= 127)
#define IS_STACKFRAME_same_locals_1_stack_item_frame_extended(num) ((num) == 247)
#define IS_STACKFRAME_chop_frame(num) ((num) >= 248&& (num) <= 250)
#define IS_STACKFRAME_same_frame_extended(num) ((num) == 251)
#define IS_STACKFRAME_append_frame(num) ((num) >= 252 && (num) <= 254)
#define IS_STACKFRAME_full_frame(num) ((num) == 255)


#endif //CJVM_CLASSFILE_H

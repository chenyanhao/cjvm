//
// Created by cyh on 2018/6/15.
//

#ifndef CJVM_JAVATYPE_H
#define CJVM_JAVATYPE_H

#include <cstdint>

class JavaClass;

class JType {
public:
    virtual ~JType() = default;
};


class Jvoid : public JType {};


class JDouble : public JType {
public:
    explicit JDouble() = default;
    explicit JDouble(double val) : val(val) {}
    double val = 0.0;
};

class JFloat : public JType {
public:
    explicit JFloat() = default;
    explicit JFloat(float val) : val(val) {}
    float val = 0.0F;
};


class JInt : public JType {
public:
    explicit JInt() = default;
    explicit JInt(int32_t val) : val(val) {}
    int32_t val = 0;
};

class JLong : public JType {
public:
    explicit JLong() = default;
    explicit JLong(int64_t val) : val(val) {}
    int64_t val = 0L;
};


class JObject : public JType {
public:
    explicit JObject() = default;

    // 在 Java 堆上的偏移量
    std::size_t offset = 0;

    // 指向实际 Java 对象的指针
    const JavaClass *jc{};
};

class JArray : public JType {
public:
    explicit JArray() = default;

    // Java 数组的长度
    int length = 0;
    // 在 Java 堆上的偏移量
    std::size_t offset = 0;
};


#define IS_COMPUTATIONAL_TYPE_1(value) \
    (typeid(*value) != typeid(JDouble) && typeid(*value) != typeid(JLong))

#define IS_COMPUTATIONAL_TYPE_2(value) \
    (typeid(*value) == typeid(JDouble) || typeid(*value) == typeid(JLong))


#define IS_JINT(x) (typeid(*x)==typeid(JInt))
#define IS_JLong(x) (typeid(*x)==typeid(JLong))
#define IS_JDouble(x) (typeid(*x)==typeid(JDouble))
#define IS_JFloat(x) (typeid(*x)==typeid(JFloat))
#define IS_JObject(x) (typeid(*x)==typeid(JObject))
#define IS_JArray(x) (typeid(*x)==typeid(JArray))


#endif //CJVM_JAVATYPE_H

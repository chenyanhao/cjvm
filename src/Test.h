//
// Created by cyh on 2018/6/5.
//

#ifndef CJVM_TEST_H
#define CJVM_TEST_H


#include <cstdint>

#include <iostream>
#include <stdio.h>
#include "Type.h"

class Test {

};


class ConstantTag {

public:
    static const ConstantTag TAG_Utf8;

private:
    uint8_t value;
private:
    ConstantTag(uint8_t value) : value(value) {}
};

const ConstantTag ConstantTag::TAG_Utf8 = ConstantTag(1);


#endif //CJVM_TEST_H

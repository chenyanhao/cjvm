//
// Created by cyh on 2018/5/31.
//

#ifndef CJVM_FILEREADER_H
#define CJVM_FILEREADER_H

#include <fstream>
#include "Type.h"

class FileReader {
public:
    FileReader() = default;

    FileReader(const std::string filePath) : fin(filePath, std::ios::binary) {
        this->filePath = filePath;
    }

    ~FileReader() {
        fin.close();
    }

    bool openFile(const std::string filePath) {
        if (!fin.is_open()) {
            fin.open(filePath, std::ios::binary);
            return fin.is_open();
        }
        return true;
    }

    bool hasNoExtraBytes() {
        return fin.peek() == EOF;
    }


    u4 readU4() {
        fin.read(u4buf, 4);
        return getu4(u4buf);
    }

    u2 readU2() {
        fin.read(u2buf, 2);
        return getu2(u2buf);
    }

    u1 readU1() {
        fin.read(u1buf, 1);
        return getu1(u1buf);
    }

private:
    std::ifstream fin;
    std::string filePath;
    char u4buf[4];
    char u2buf[2];
    char u1buf[1];
};

#endif //CJVM_FILEREADER_H

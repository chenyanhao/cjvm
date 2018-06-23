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
public:
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


class DbgPleasant {
public:
    DbgPleasant(std::string title, int columnCount) : title(std::move(title)), column(columnCount) {
        cells.emplace_back();
    }

    void setCellWidth(int w) {
        this->columnWidth = w;
    }

    void addCell(const std::string &str) {
        std::string nstr;
        if (str.length() > columnWidth - 2) {
            nstr = str.substr(0, columnWidth - 2);
        } else {
            nstr = str;
        }

        if (currentColumn == column) {
            cells.emplace_back();
            currentColumn = 0;
        }

        cells.at(cells.size() - 1).push_back(nstr);
        ++currentColumn;
    }

    void show() {
        // header
        for (int i = 0; i < columnWidth * column; ++i) {
            std::cout << "-";
        }
        std::cout << "\n";
        unsigned long long int margin = ((column * columnWidth) - 2 - title.length()) / 2;
        std::cout << "/";
        for (int i = 0; i < margin; ++i) {
            std::cout << " ";
        };
        std::cout << title;
        for (int i = 0; i < margin; ++i) {
            std::cout << " ";
        }
        std::cout << "/\n";
        for (int i = 0; i < columnWidth * column; ++i) {
            std::cout << "-";
        }
        std::cout << "\n";

        //Body
        for (auto &cell : cells) {
            for (auto &k : cell) {
                std::cout << "|";
                std::cout << k;
                for (int p = 0; (columnWidth - 2 - k.length()) > 0 && p < (columnWidth - 2 - k.length()); ++p) {
                    std::cout << " ";
                }
                std::cout << "|";
            }
            std::cout << "\n";
        }

        //End
        for (int i = 0; i < columnWidth * column; i++) {
            std::cout << "-";
        }
        std::cout << "\n";
    }

private:
    std::string title;
    std::deque<std::deque<std::string>> cells;
    int currentColumn = 0;
    int column;
    int columnWidth = 30;
};



#endif //CJVM_DEBUG_H

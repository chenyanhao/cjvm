//
// Created by ha on 18/6/16.
//

#include "MethodArea.h"
#include "JavaClass.h"
#include "Option.h"
#include "AccessFlag.h"
#include "Descriptor.h"


MethodArea::MethodArea(const std::vector<std::string> &libPaths) {
    for (const auto &path : libPaths) {
        searchPaths.push_back(path);
    }
}

MethodArea::~MethodArea() {
    for (auto &x : classTable) {
        delete x.second;
    }
}

JavaClass* MethodArea::findJavaClass(const char *javaClassName) {
    std::lock_guard<std::recursive_mutex> lockMA(maMutex);

    const auto pos = classTable.find(javaClassName);
    if (pos != classTable.end()) {
        return pos->second;
    }

    return nullptr;
}


bool MethodArea::loadJavaClass(const char *javaClassName) {
    std::lock_guard<std::recursive_mutex> lockMA(maMutex);

    auto path = parseName2Path(javaClassName);
    if (path.length() != 0 && !findJavaClass(javaClassName)) {
        JavaClass *jc = new JavaClass(path.c_str());
        // TODO
    }
}






cmake_minimum_required(VERSION 3.6)
project(cjvm)

set(CMAKE_CXX_STANDARD 11)

add_compile_options(-O1)

set(CMAKE_EXE_LINKER_FLAGS "-lpthread")

set(SOURCE_FILES
        src/Type.h src/Util.h src/ClassFile.h "src/FileReader.h" src/JavaType.h src/AccessFlag.h
        src/Concurrent.cpp src/Concurrent.hpp src/Option.h src/Frame.h src/Descriptor.cpp src/Descriptor.h
        src/Opcode.h src/JavaException.cpp src/JavaException.h src/ObjectMonitor.cpp src/ObjectMonitor.h
        src/RuntimeEnv.cpp src/RuntimeEnv.h src/MethodArea.cpp src/MethodArea.h src/JavaClass.cpp src/JavaClass.h src/Debug.cpp src/Debug.h)
add_executable(cjvm ${SOURCE_FILES})

target_link_libraries(cjvm pthread)

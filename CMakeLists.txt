cmake_minimum_required(VERSION 3.8)
project(cjvm)

set(CMAKE_CXX_STANDARD 11)

add_compile_options(-O1)

set(CMAKE_EXE_LINKER_FLAGS "-lpthread")

set(SOURCE_FILES src/Type.h src/Util.h src/ClassFile.h "src/FileReader .h" src/Test.cpp src/Test.h)
add_executable(cjvm ${SOURCE_FILES})

target_link_libraries(cjvm pthread)

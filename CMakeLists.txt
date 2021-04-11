cmake_minimum_required(VERSION 3.12)
project(CPU LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

find_package(LLVM 10.0.0 REQUIRED CONFIG)
include_directories(${LLVM_INCLUDE_DIRS})
add_definitions(${LLVM_DEFINITIONS})

set(CMAKE_CXX_FLAGS "-Wall -Wextra")
set(CMAKE_CXX_FLAGS_DEBUG "-g -DOK_DUMP")
set(CMAKE_CXX_FLAGS_RELEASE "-O2 -DNDEBUG")

add_executable(asm asm.cpp Parser.h)
add_executable(cpu cpu.cpp)
add_executable(disasm DisAsm.cpp)
add_executable(trans translator_x86_64.cpp translator_x86_64.h My_Headers/elf_maker.h)

add_library(txt_files STATIC My_Headers/txt_files.c My_Headers/txt_files.h)
add_library(stack STATIC My_Headers/protected_stack.c My_Headers/protected_stack.h)
add_library(libcpu STATIC cpuStruct.cpp cpuStruct.h)
add_library(common STATIC linker.h)

set_target_properties(common PROPERTIES LINKER_LANGUAGE CXX)
set_target_properties(txt_files PROPERTIES LINKER_LANGUAGE CXX)
set_target_properties(stack PROPERTIES LINKER_LANGUAGE CXX)

llvm_map_components_to_libnames(
        llvm_libs
        Analysis
        Core
        ExecutionEngine
        InstCombine
        Object
        OrcJIT
        RuntimeDyld
        ScalarOpts
        Support
        TransformUtils
        native
        irreader
)

target_link_libraries(asm txt_files common)
target_link_libraries(disasm txt_files common)
target_link_libraries(cpu common stack txt_files libcpu)
target_link_libraries(trans txt_files common)
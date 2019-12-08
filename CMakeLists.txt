cmake_minimum_required(VERSION 3.15)
project(Processor_Laba)

set(CMAKE_CXX_STANDARD 14)

add_executable(Processor_Laba_asm asm.cpp Parser.h)
add_executable(Processor_Laba_cpu cpu.cpp)
add_executable(Processor_Laba_disasm DisAsm.cpp)
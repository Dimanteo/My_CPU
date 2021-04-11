CC = clang++
CXX = clang++
LLVM_DEPS = `llvm-config --cxxflags --ldflags --libs`
CFLAGS := -Wall -Wextra -g -MD
CXXFLAGS := $(CFLAGS)
TXT_LIB := ../My_Headers/txt_files.o

all: Instruction.o Core.o Tracer.o exec.o generate.o LLVM_IR.o $(TXT_LIB)
	$(CC) $^ $(LLVM_DEPS) -o llcpu.out

ram: Instruction.o Core.o RAMTracer.o exec.o generate.o LLVM_IR.o $(TXT_LIB)
	$(CC) $^ $(LLVM_DEPS) -o llcpu.out

Instruction.o : Instruction.cpp

Core.o : Core.cpp

LLVM_IR.o : LLVM_IR.cpp

Tracer.o : Tracer.cpp

RAMTracer.o : Tracer.cpp
	$(CXX) $(CXXFLAGS) -DMEM_DUMP -c $^ -o $@

exec.o : exec.cpp

generate.o : generate.cpp

.PHONY: clean

clean:
	rm -f *.o *.d

-include *.d
CC = clang++
LLVM_DEPS = `llvm-config --cxxflags --ldflags --libs`
CFLAGS := -Wall -Wextra -g -MD -c
CXXFLAGS := $(CFLAGS)
TXT_LIB := ../My_Headers/txt_files.o

all: Instruction.o Core.o LLVM_IR.o $(TXT_LIB)
	$(CC) $^ $(LLVM_DEPS) -o llcpu.out

Instruction.o : Instruction.cpp

Core.o : Core.cpp

LLVM_IR.o : LLVM_IR.cpp

.PHONY: clean

clean:
	rm -f *.o *.d

-include *.d
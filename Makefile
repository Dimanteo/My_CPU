CC := g++
CXX := $(CC)
CFLAGS := -g -Wall -MD -DNDEBUG
CXXFLAGS := $(CFLAGS)
TXT_LIB := My_Headers/txt_files.o

all: asm.elf dis.elf cpu.elf trans.elf

.PHONY: LLVM
LLVM:
	$(MAKE) -C LLVM/

asm.elf : asm.o $(TXT_LIB)
	$(CXX) $^ -o $@

dis.elf : DisAsm.o $(TXT_LIB)
	$(CXX) $^ -o $@

cpu.elf : cpu.o cpuStruct.o $(TXT_LIB) My_Headers/protected_stack.o
	$(CXX) $^ -o $@

trans.elf : translator_x86_64.o $(TXT_LIB)
	$(CXX) $^ -o $@

asm.o : asm.cpp

DisAsm.o : DisAsm.cpp

cpu.o : cpu.cpp

cpuStruct.o : cpuStruct.cpp

translator_x86_64.o : translator_x86_64.cpp

.PHONY: clean
clean:
	rm -f *.o *.d *.out *.elf

-include *.d
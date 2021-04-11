#pragma once
#include <iostream>
#include <llvm-10/llvm/ExecutionEngine/ExecutionEngine.h>
#include <llvm-10/llvm/ExecutionEngine/GenericValue.h>
#include <llvm-10/llvm/IR/BasicBlock.h>
#include <llvm-10/llvm/IR/IRBuilder.h>
#include <llvm-10/llvm/IR/LLVMContext.h>
#include <llvm-10/llvm/IR/Module.h>
#include <llvm-10/llvm/Support/TargetSelect.h>
#include <unordered_map>
#include <vector>

#include "../linker.h"
#include "Tracer.hpp"
#include "types.hpp"

class Core {
    friend class Tracer;
    std::vector<word_t> m_stack;
    reg_t regFile[NREGS];
    size_t m_pc;
    char *m_memory;
    bool m_running;
    std::unordered_map<size_t, llvm::BasicBlock *> bblockCache;
    llvm::Module *m_module;
    Tracer *m_tracer;

  public:
    Core();
    ~Core();
    void run(char *code, size_t codeOffset, size_t codeSz);
    void push(word_t val);
    word_t pop();
    reg_t getReg(REG_CODE regi) const;
    void setReg(REG_CODE regi, reg_t val);
    void read(address_t adr, size_t nbytes, char *dest) const;
    void readWord(address_t adr, word_t *dest) const;
    void write(address_t adr, size_t nbytes, char *src);
    void writeWord(address_t adr, word_t src) const;
    void stop();
    llvm::Module *getModule() const;
    Tracer *getTracer() const;
    void assignTracer(Tracer *tracer);
    llvm::BasicBlock *getBasicBlock(size_t pc);
    size_t getPC() const;
    void setPC(size_t newPC);
};
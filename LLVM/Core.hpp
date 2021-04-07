#pragma once
#include <vector>
#include <unordered_map>
#include <iostream>
#include <llvm-10/llvm/IR/LLVMContext.h>
#include <llvm-10/llvm/IR/Module.h>
#include <llvm-10/llvm/IR/IRBuilder.h>
#include <llvm-10/llvm/IR/BasicBlock.h>
#include <llvm-10/llvm/ExecutionEngine/ExecutionEngine.h>
#include <llvm-10/llvm/ExecutionEngine/GenericValue.h>
#include <llvm-10/llvm/Support/TargetSelect.h>

#include "../linker.h"

using word_t = int;
using reg_t = int;
using address_t = size_t;

class Core
{
        std::vector<word_t> m_stack;
        reg_t regFile[NREGS];
        size_t m_pc;
        word_t *m_memory;
        bool m_running;
        std::unordered_map<size_t, llvm::BasicBlock*> bblockCache;

    public:
        Core();
        ~Core();
        void run(char *code, size_t codeOffset, size_t codeSz);
        void push(word_t val);
        word_t pop();
        reg_t getReg(REG_CODE regi) const;
        void setReg(REG_CODE regi);
        void read(address_t adr) const;
        void write(address_t adr);
        void stop();
};
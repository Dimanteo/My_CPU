#include "Core.hpp"
#include "Instruction.hpp"

Core::Core() : m_memory(new word_t [RAM_SIZE]), m_running(false) {};

Core::~Core() {
    delete [] (m_memory);
}

void Core::run(char *code, size_t codeOffset, size_t codeSz) {
    char *entry = code + codeOffset;

    llvm::LLVMContext context;
    llvm::Module module("top", context);
    llvm::IRBuilder<> builder(context);

    llvm::FunctionType *funcType = 
        llvm::FunctionType::get(builder.getVoidTy(), false);
    llvm::Function *mainFunc = 
        llvm::Function::Create(funcType, llvm::Function::ExternalLinkage, "main", module);
    llvm::BasicBlock *startBB = llvm::BasicBlock::Create(context, "start", mainFunc);

    m_bbCache.insert({codeOffset, startBB});
    builder.SetInsertPoint(startBB);

    // Decode and generate bblock map
    Insn insn;
    char *nextPC;
    std::vector<Insn> decodedInsns;
    for (char *pc = entry; pc - entry < codeSz; pc = nextPC) {
        insn.decode(pc);
        decodedInsns.push_back(insn);
        nextPC = pc + sizeof(char) + insn.argc() * sizeof(word_t);
        if (insn.isBranch()) {
            // Creating new BB for jump destination
            size_t dest = insn.getArg(0);
            if (m_bbCache.find(dest) != m_bbCache.end()) {
                llvm::BasicBlock *destBB = llvm::BasicBlock::Create(context, std::to_string(dest), mainFunc);
                m_bbCache.insert({dest, destBB});
            }
            // End current basic block and switch to new one.
            size_t offset = nextPC - code;
            if (m_bbCache.find(offset) != m_bbCache.end()) {
                llvm::BasicBlock *nextBB = llvm::BasicBlock::Create(context, std::to_string(offset), mainFunc);
                m_bbCache.insert({offset, nextBB});
            }
        }
    }

    m_running = true;
    // ExecutionEngine
}

#include "Core.hpp"
#include "Instruction.hpp"

Core::Core()
    : m_memory(new char[RAM_SIZE * sizeof(word_t)]), m_running(false){};

Core::~Core() { delete[](m_memory); }

void Core::run(char *code, size_t codeOffset, size_t codeSz) {
    char *entry = code + codeOffset;
    llvm::LLVMContext context;
    llvm::Module module("top", context);
    llvm::IRBuilder<> builder(context);
    llvm::FunctionType *funcType =
        llvm::FunctionType::get(builder.getVoidTy(), false);
    llvm::Function *mainFunc = llvm::Function::Create(
        funcType, llvm::Function::ExternalLinkage, "main", module);
    llvm::BasicBlock *startBB =
        llvm::BasicBlock::Create(context, "start", mainFunc);

    bblockCache.insert({codeOffset, startBB});

    // First pass: create basic block map
    Insn insn;
    char *nextPC;
    std::vector<std::pair<size_t, Insn>> decodedInsns;
    for (char *pc = entry; pc - entry < codeSz; pc = nextPC) {
        insn.decode(pc);
        decodedInsns.push_back({pc - code, insn});
        nextPC = pc + insn.getSz();
        if (insn.isBranch()) {
            // Creating new basic block for jump destination
            size_t dest = insn.getArg(0);
            if (bblockCache.find(dest) != bblockCache.end()) {
                llvm::BasicBlock *destBB = llvm::BasicBlock::Create(
                    context, std::to_string(dest), mainFunc);
                bblockCache.insert({dest, destBB});
            }
            // End current basic block and switch to new one.
            size_t offset = nextPC - code;
            if (bblockCache.find(offset) != bblockCache.end()) {
                llvm::BasicBlock *nextBB = llvm::BasicBlock::Create(
                    context, std::to_string(offset), mainFunc);
                bblockCache.insert({offset, nextBB});
            }
        }
    }

    // Second pass: generating IR
    for (auto pc_insn : decodedInsns) {
        if (bblockCache.find(pc_insn.first) != bblockCache.end())
            builder.SetInsertPoint(bblockCache[pc_insn.first]);
        pc_insn.second.generateIR(&builder, *this);
    }

    // Printing for debug
    std::cout << "## [LLVM IR] DUMP ##\n";
    std::string ir_str;
    llvm::raw_string_ostream ir_os(ir_str);
    module.print(ir_os, nullptr);
    ir_os.flush();
    std::cout << ir_str;
    llvm::InitializeNativeTarget();
    llvm::InitializeNativeTargetAsmPrinter();

    // Execute IR
    llvm::ExecutionEngine *ee =
        llvm::EngineBuilder(std::unique_ptr<llvm::Module>(&module)).create();
    ee->InstallLazyFunctionCreator(lazyFunctionCreator);
    ee->finalizeObject();
    std::vector<llvm::GenericValue> noargs;
    m_running = true;
    ee->runFunction(mainFunc, noargs);
}

void Core::push(word_t val) { m_stack.push_back(val); }

word_t Core::pop() {
    word_t val = m_stack.back();
    m_stack.pop_back();
    return val;
}

reg_t Core::getReg(REG_CODE regi) const { return regFile[regi]; }

void Core::setReg(REG_CODE regi, reg_t val) { regFile[regi] = val; }

void Core::read(address_t adr, size_t nbytes, char *dest) const {
    memcpy(dest, m_memory + adr, nbytes);
}

void Core::readWord(address_t adr, word_t *dest) const {
    *dest = reinterpret_cast<word_t *>(m_memory)[adr];
}

void Core::write(address_t adr, size_t nbytes, char *src) {
    memcpy(m_memory + adr, src, nbytes);
}

void Core::writeWord(address_t adr, word_t src) const {
    reinterpret_cast<word_t *>(m_memory)[adr] = src;
}

void Core::stop() { m_running = false; }

#include "Core.hpp"
#include "Instruction.hpp"
#include "exec.hpp"
#include "generate.hpp"

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

    m_module = &module;
    bblockCache.insert({codeOffset, startBB});
    builder.SetInsertPoint(startBB);

    // First pass: create basic block map
    Insn insn;
    char *nextPC;
    std::vector<std::pair<size_t, Insn>> decodedInsns;
    for (char *pc = entry; pc < entry + codeSz; pc = nextPC) {
        insn.decode(pc);
        decodedInsns.push_back({pc - code, insn});
        nextPC = pc + insn.getSz();
        if (insn.isBranch() && insn.getCode() != CMD_END) {
            // Creating new basic block for jump destination
            size_t dest = insn.getArg(0);
            if (bblockCache.find(dest) == bblockCache.end()) {
                llvm::BasicBlock *destBB = llvm::BasicBlock::Create(
                    context, std::to_string(dest), mainFunc);
                bblockCache.insert({dest, destBB});
            }
            // End current basic block and switch to new one.
            size_t offset = nextPC - code;
            if (bblockCache.find(offset) == bblockCache.end()) {
                llvm::BasicBlock *nextBB = llvm::BasicBlock::Create(
                    context, std::to_string(offset), mainFunc);
                bblockCache.insert({offset, nextBB});
            }
        }
    }

    // Second pass: generating IR
    for (auto &pc_insn : decodedInsns) {
        m_pc = pc_insn.first;
        pc_insn.second.generateIR(&builder, *this);
        auto nextBBlock_it =
            bblockCache.find(pc_insn.first + pc_insn.second.getSz());
        if (nextBBlock_it != bblockCache.end()) {
            if (!pc_insn.second.isBranch())
                builder.CreateBr((*nextBBlock_it).second);
            builder.SetInsertPoint((*nextBBlock_it).second);
        }
    }

    functionCreatorMap.insert({"pop", reinterpret_cast<void *>(ir_pop)});

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
    ee->InstallLazyFunctionCreator(Insn::lazyFunctionCreator);
    ee->finalizeObject();
    std::vector<llvm::GenericValue> noargs;
    m_running = true;
    m_pc = codeOffset;
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

llvm::Module *Core::getModule() const { return m_module; }

Tracer *Core::getTracer() const { return m_tracer; }

void Core::assignTracer(Tracer *tracer) {
    m_tracer = tracer;
    m_tracer->watch(this);
    functionCreatorMap.insert({"trace", reinterpret_cast<void *>(trace)});
}

llvm::BasicBlock *Core::getBasicBlock(size_t pc) { return bblockCache[pc]; }

size_t Core::getPC() const { return m_pc; }

void Core::setPC(size_t newPC) { m_pc = newPC; }
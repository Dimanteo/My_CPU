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
                llvm::BasicBlock *destBB = llvm::BasicBlock::Create(context, std::to_string(dest), mainFunc);
                bblockCache.insert({dest, destBB});
            }
            // End current basic block and switch to new one.
            size_t offset = nextPC - code;
            if (bblockCache.find(offset) != bblockCache.end()) {
                llvm::BasicBlock *nextBB = llvm::BasicBlock::Create(context, std::to_string(offset), mainFunc);
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

    std::cout << "## [LLVM IR] DUMP ##\n";

    std::string ir_str;
    llvm::raw_string_ostream ir_os(ir_str);
    module.print(ir_os, nullptr);
    ir_os.flush();
    std::cout << ir_str;

    std::cout << "\n## [LLVM EE] RUN ##\n";

    llvm::InitializeNativeTarget();
    llvm::InitializeNativeTargetAsmPrinter();

    std::cout << "## [LLVM EE] END ##\n";

    llvm::ExecutionEngine *ee = llvm::EngineBuilder(std::unique_ptr<llvm::Module>(&module)).create();
    ee->InstallLazyFunctionCreator(lazyFunctionCreator);
    ee->finalizeObject();
    std::vector<llvm::GenericValue> noargs;
    m_running = true;
    ee->runFunction(mainFunc, noargs);
}

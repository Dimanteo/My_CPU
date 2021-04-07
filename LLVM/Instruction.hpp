#pragma once
#include <llvm-10/llvm/IR/IRBuilder.h>
#include <string>
#include <unordered_map>
#include <vector>

#include "../linker.h"
#include "types.hpp"

class Core;

class Insn {
    CMD_CODE m_code;
    bool m_isBranch;
    int m_argc;
    word_t m_argv[MAX_NUM_ARGS];
    std::string execFName;

    using execFunc_t = void (*)(Core *);
    execFunc_t m_exec;
    static std::unordered_map<std::string, execFunc_t> functionCreatorMap;

    word_t fetchArg(const char *pc, int pos);

  public:
    void generateIR(llvm::IRBuilder<> *builder, const Core &core) const;
    void exec(Core *core) const;
    void decode(const char *pc);
    bool isBranch() const;
    int argc() const;
    word_t getArg(int argi) const;
    size_t getSz() const;
    static void *lazyFunctionCreator(const std::string fname);
};

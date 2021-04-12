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
    bool m_isTerminator;
    int m_argc;
    word_t m_argv[MAX_NUM_ARGS];
    std::string execFName;
    word_t fetchArg(const char *pc, int pos);

  public:
    using execFunc_t = void (*)(Core *, const Insn *);
    using genFunc_t = void (*)(llvm::IRBuilder<> *, Core &, const Insn &);

  private:
    execFunc_t m_exec;
    genFunc_t m_genIR;

  public:
    void generateIR(llvm::IRBuilder<> *builder, Core &core);
    void exec(Core *core) const;
    void decode(const char *pc);
    bool isBranch() const;
    bool isTerm() const;
    int argc() const;
    word_t getArg(int argi) const;
    size_t getSz() const;
    CMD_CODE getCode() const;
    std::string getName() const;
    static void *lazyFunctionCreator(const std::string fname);
    static const char *getToken(CMD_CODE code);
};

#pragma once
#include <vector>

#include "Instruction.hpp"
#include "types.hpp"

class Core;

class Tracer {
    Core *m_core;
    FILE *m_sout = stdout;

    using cnt_t = size_t;
    size_t m_dumpCnt;
    cnt_t insnExecuted;
    cnt_t branches;

    cnt_t execs_cnt[ISA_POWER];

    void statDump(const char *prefix) const;
    void registerDump(const char *prefix) const;
    void stackDump(const char *prefix) const;
    void memoryDump() const;

  public:
    void setOutputStream(FILE *stream);
    void watch(Core *core);
    void stat(const Insn &insn);
    void dump(const Insn &insn);
};
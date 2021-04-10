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
    // cnt_t brTaken;
    // cnt_t brNotTaken;

    cnt_t execs_cnt[ISA_POWER];

    void statDump() const;
    void registerDump() const;
    void stackDump() const;
    void memoryDump() const;

  public:
    void setOutputStream(FILE *stream);
    void watch(Core *core);
    void stat(const Insn &insn);
    void dump(const Insn &insn);
};
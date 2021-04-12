#include <stdio.h>

#include "Core.hpp"
#include "Tracer.hpp"

void Tracer::setOutputStream(FILE *stream) { m_sout = stream; }

void Tracer::watch(Core *core) {
    m_dumpCnt = 0;
    m_core = core;
    insnExecuted = 0;
    branches = 0;
    for (size_t i = 0; i < ISA_POWER; i++) {
        execs_cnt[i] = 0;
    }
}

void Tracer::stat(const Insn &insn) {
    insnExecuted++;
    if (insn.isBranch()) {
        branches++;
    }
    execs_cnt[insn.getCode()]++;
}

void Tracer::dump(const Insn &insn) {
    fprintf(m_sout, "## Core state dump %lu ##\n{\n", ++m_dumpCnt);
    fprintf(m_sout, "\tInstruction: %s\n", insn.getName().c_str());
    statDump();
    registerDump();
    stackDump();
    memoryDump();
    fprintf(m_sout, "}\n");
}

void Tracer::statDump() const {
    fprintf(m_sout, "\tInstructions summary:\n");
    fprintf(m_sout, "\tExecuted: %lu\n", insnExecuted);
    fprintf(m_sout, "\tBranches: %lu\n", branches);
    fprintf(m_sout, "\tBy instruction:\n");
    for (int i = 0; i < ISA_POWER; i++) {
        fprintf(m_sout, "\t\t[%d] : %lu\n", i, execs_cnt[i]);
    }
}

void Tracer::registerDump() const {
    fprintf(m_sout, "\tRegister file:\n");
    for (unsigned int i = 0; i < NREGS; i++) {
        fprintf(m_sout, "\t\t%cX : [%d]\n", 'A' + i,
                m_core->getReg(static_cast<REG_CODE>(i)));
    }
}
void Tracer::stackDump() const {
    fprintf(m_sout, "\tStack dump:\n\t{\n");
    for (word_t item : m_core->m_stack) {
        fprintf(m_sout, "\t\t%d\n", item);
    }
    fprintf(m_sout, "\t}\n");
}

void Tracer::memoryDump() const {
#ifdef MEM_DUMP
    fprintf(m_sout, "\tMemory dump\n");
    size_t lineLength = 20;
    for (size_t adr = 0; adr < RAM_SIZE; adr++) {
        if (adr % lineLength == 0)
            fprintf(m_sout, "\t\t");
        fprintf(m_sout, "[%lu]%d ", adr, ((word_t*)m_core->m_memory)[adr]);
        if (adr % lineLength == 0)
            fprintf(m_sout, "\n");
    }
    fprintf(m_sout, "\n");
#endif
}
#include <stdio.h>

#include "Core.hpp"
#include "Tracer.hpp"

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
    printf("## Core state dump %lu ##\n{\n", ++m_dumpCnt);
    printf("\tInstruction: %s\n", insn.getName().c_str());
    statDump();
    registerDump();
    stackDump();
    memoryDump();
    printf("}\n");
}

void Tracer::statDump() const {
    printf("\tInstructions summary:\n");
    printf("\tExecuted: %lu\n", insnExecuted);
    printf("\tBranches: %lu\n", branches);
    printf("\tBy instruction:\n");
    for (int i = 0; i < ISA_POWER; i++) {
        printf("\t\t[%d] : %lu\n", i, execs_cnt[i]);
    }
    
}

void Tracer::registerDump() const {
    printf("\tRegister file:\n");
    for (unsigned int i = 0; i < NREGS; i++) {
        printf("\t\t%cX : [%d]\n", 'A' + i,
               m_core->getReg(static_cast<REG_CODE>(i)));
    }
}
void Tracer::stackDump() const {
    printf("\tStack dump:\n\t{\n");
    for (word_t item : m_core->m_stack) {
        printf("\t\t%d\n", item);
    }
    printf("\t}\n");
}

void Tracer::memoryDump() const {
#ifdef MEM_DUMP
    printf("\tMemory dump\n");
    size_t lineLength = 20;
    for (size_t adr = 0; adr < RAM_SIZE; adr++) {
        if (adr % lineLength == 0)
            printf("\t\t");
        printf("[%lu]%d ", adr, m_core->m_memory[adr]);
        if (adr % lineLength == 0)
            printf("\n");
    }
    printf("\n");
#endif
}
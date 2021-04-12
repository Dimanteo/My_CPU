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
    fprintf(m_sout, "{\n");
    fprintf(m_sout, "\t\"number\": %lu,\n", m_dumpCnt);
    fprintf(m_sout, "\t\"name\": %s,\n", insn.getName().c_str());
    statDump("\t");
    registerDump("\t");
    stackDump("\t");
    // memoryDump();
    fprintf(m_sout, "}\n\n");
}

void Tracer::statDump(const char *prefix) const {
    fprintf(m_sout, "%s\"executed\": %lu,\n", prefix, insnExecuted);
    fprintf(m_sout, "%s\"branches\": %lu,\n", prefix, branches);
    fprintf(m_sout, "%s\"by_insn\": {\n", prefix);
    for (int i = 0; i < ISA_POWER; i++) {
        fprintf(m_sout, "%s\t%10s : %lu,\n", prefix,
                Insn::getToken(static_cast<CMD_CODE>(i)), execs_cnt[i]);
    }
    fprintf(m_sout, "%s},\n", prefix);
}

void Tracer::registerDump(const char *prefix) const {
    fprintf(m_sout, "%s\"RegFile\" : {\n", prefix);
    for (unsigned int i = 0; i < NREGS; i++) {
        fprintf(m_sout, "%s\t\"%cX\" : %d,\n", prefix, 'A' + i,
                m_core->getReg(static_cast<REG_CODE>(i)));
    }
    fprintf(m_sout, "%s},\n", prefix);
}
void Tracer::stackDump(const char *prefix) const {
    fprintf(m_sout, "%s\"stack\" : [", prefix);
    for (word_t item : m_core->m_stack) {
        fprintf(m_sout, "%d, ", item);
    }
    fprintf(m_sout, "]\n");
}

void Tracer::memoryDump() const {
    fprintf(m_sout, "\tRead Field\n");
    size_t lineLength = 100;
    for (size_t r = 0; r < lineLength; r++) {
        for (size_t c = 0; c < lineLength; c++) {
            fprintf(m_sout, "%c",
                    m_core->m_memory[r * lineLength + c] ? '+' : 'o');
        }
        fprintf(m_sout, "\n");
    }
    fprintf(m_sout, "\nWrite Field\n");
    for (size_t r = 0; r < lineLength; r++) {
        for (size_t c = 0; c < lineLength; c++) {
            fprintf(m_sout, "%c",
                    m_core->m_memory[10000 + r * lineLength + c] ? '+' : 'o');
        }
        fprintf(m_sout, "\n");
    }
    fprintf(m_sout, "\n");
}
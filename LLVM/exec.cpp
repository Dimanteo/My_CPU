#include "exec.hpp"
#include "Instruction.hpp"
#include "Core.hpp"
#include "Tracer.hpp"
#include "types.hpp"

void trace(Core* core, const Insn &insn) {
    core->getTracer()->stat(insn);
    core->getTracer()->dump(insn);
}

void do_end(Core *core, const Insn &insn) {
    core->stop();
}

void do_push(Core *core, const Insn &insn) {
    core->push(insn.getArg(0));
}

void do_pop(Core *core, const Insn &insn) {
    core->setReg(static_cast<REG_CODE>(insn.getArg(0)), core->pop());
}

void do_out(Core *core, const Insn &insn) {
    printf("%g\n", static_cast<float>(core->pop()) / PRECISION);
}

void do_in(Core *core, const Insn &insn) {
    float val = 0;
    scanf("%f", &val);
    core->push(static_cast<word_t>(val * PRECISION));
}

void do_pushx(Core *core, const Insn &insn) {
    core->push(core->getReg(static_cast<REG_CODE>(insn.getArg(0))));
}
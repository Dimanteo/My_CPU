#pragma once
#include "Instruction.hpp"
#include "Core.hpp"
#include "types.hpp"

void do_end(Core *core, const Insn &insn) {
    core->stop();
}

void do_push(Core *core, const Insn &insn) {
    core->push(insn.getArg(0));
}

#pragma once

class Core;
class Insn;

void trace(Core *core, const Insn *insn);

void do_end(Core *core, const Insn *insn);

void do_push(Core *core, const Insn *insn);

void do_pop(Core *core, const Insn *insn);

void do_out(Core *core, const Insn *insn);

void do_in(Core *core, const Insn *insn);

void do_pushx(Core *core, const Insn *insn);
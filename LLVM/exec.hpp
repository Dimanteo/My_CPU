#pragma once
#include "types.hpp"
#include <unordered_map>

class Core;
class Insn;

extern std::unordered_map<std::string, void *> functionCreatorMap;

void trace(Core *core, const Insn *insn);

word_t ir_pop(Core *core);

void do_end(Core *core, const Insn *insn);

void do_push(Core *core, const Insn *insn);

void do_pop(Core *core, const Insn *insn);

void do_out(Core *core, const Insn *insn);

void do_in(Core *core, const Insn *insn);

void do_pushx(Core *core, const Insn *insn);

void do_add(Core *core, const Insn *insn);

void do_sub(Core *core, const Insn *insn);

void do_mul(Core *core, const Insn *insn);

void do_div(Core *core, const Insn *insn);

void do_sqr(Core *core, const Insn *insn);

void do_sin(Core *core, const Insn *insn);

void do_cos(Core *core, const Insn *insn);

void do_power(Core *core, const Insn *insn);
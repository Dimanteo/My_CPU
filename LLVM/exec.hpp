#pragma once
#include "types.hpp"
#include <unordered_map>

class Core;
class Insn;

extern std::unordered_map<std::string, void *> functionCreatorMap;

void trace(Core *core, const Insn *insn);

word_t ir_pop(Core *core);

void ir_push(Core *core, word_t val);

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

void do_load_xn(Core *core, const Insn *insn); // pushram_xn

void do_load_nx(Core *core, const Insn *insn); // pushram_nx

void do_load_x(Core *core, const Insn *insn); // pushram_x

void do_store_x(Core *core, const Insn *insn); // popram_x

void do_load(Core *core, const Insn *insn); // pushram

void do_store(Core *core, const Insn *insn); // popram

void do_store_xn(Core *core, const Insn *insn); // popram_xn

void do_store_nx(Core *core, const Insn *insn); // popram_nx
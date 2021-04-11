#include "exec.hpp"
#include "Core.hpp"
#include "Instruction.hpp"
#include "Tracer.hpp"
#include "types.hpp"

std::unordered_map<std::string, void *> functionCreatorMap;

void trace(Core *core, const Insn *insn) {
    core->getTracer()->dump(*insn);
    core->getTracer()->stat(*insn);
}

word_t ir_pop(Core *core) { return core->pop(); }

void ir_push(Core *core, word_t val) { core->push(val); }

void do_end(Core *core, const Insn *insn) { core->stop(); }

void do_push(Core *core, const Insn *insn) { core->push(insn->getArg(0)); }

void do_pop(Core *core, const Insn *insn) {
    core->setReg(static_cast<REG_CODE>(insn->getArg(0)), core->pop());
}

void do_out(Core *core, const Insn *insn) {
    printf("%g\n", static_cast<float>(core->pop()) / PRECISION);
}

void do_in(Core *core, const Insn *insn) {
    float val = 0;
    scanf("%f", &val);
    core->push(static_cast<word_t>(val * PRECISION));
}

void do_pushx(Core *core, const Insn *insn) {
    core->push(core->getReg(static_cast<REG_CODE>(insn->getArg(0))));
}

void do_add(Core *core, const Insn *insn) {
    word_t arg1 = core->pop();
    word_t arg2 = core->pop();
    core->push(arg1 + arg2);
}

void do_sub(Core *core, const Insn *insn) {
    word_t rhs = core->pop();
    word_t lhs = core->pop();
    core->push(lhs - rhs);
}

void do_mul(Core *core, const Insn *insn) {
    word_t arg1 = core->pop();
    word_t arg2 = core->pop();
    core->push(arg1 * arg2 / PRECISION);
}

void do_div(Core *core, const Insn *insn) {
    float rhs = static_cast<float>(core->pop());
    float lhs = static_cast<float>(core->pop());
    core->push(static_cast<word_t>(lhs / rhs * PRECISION));
}

void do_sqr(Core *core, const Insn *insn) {
    float val = static_cast<float>(core->pop()) / PRECISION;
    core->push(static_cast<word_t>(sqrtf(val) * PRECISION));
}

void do_sin(Core *core, const Insn *insn) {
    float val = static_cast<float>(core->pop()) / PRECISION;
    core->push(static_cast<word_t>(sinf(val) * PRECISION));
}

void do_cos(Core *core, const Insn *insn) {
    float val = static_cast<float>(core->pop()) / PRECISION;
    core->push(static_cast<word_t>(cosf(val) * PRECISION));
}

void do_power(Core *core, const Insn *insn) {
    float power = static_cast<float>(core->pop()) / PRECISION;
    double base = static_cast<double>(core->pop()) / PRECISION;
    int res = static_cast<word_t>(powf(base, power) * PRECISION);
    core->push(res);
}

void do_store_nx(Core *core, const Insn *insn) { // popram_nx
    word_t number = insn->getArg(0);
    REG_CODE regi = static_cast<REG_CODE>(insn->getArg(1));
    core->writeWord(number + core->getReg(regi) / PRECISION, core->pop());
}

void do_store_xn(Core *core, const Insn *insn) { // popram_xn
    REG_CODE regi = static_cast<REG_CODE>(insn->getArg(0));
    word_t number = insn->getArg(1);
    core->writeWord(number + core->getReg(regi) / PRECISION, core->pop());
}

void do_store(Core *core, const Insn *insn) { // popram
    core->writeWord(insn->getArg(0), core->pop());
}

void do_load(Core *core, const Insn *insn) { // pushram
    word_t word;
    core->readWord(insn->getArg(0), &word);
    core->push(word);
}

void do_store_x(Core *core, const Insn *insn) { // popram_x
    REG_CODE regi = static_cast<REG_CODE>(insn->getArg(0));
    core->writeWord(regi / PRECISION, core->pop());
}

void do_load_x(Core *core, const Insn *insn) { // pushram_x
    REG_CODE regi = static_cast<REG_CODE>(insn->getArg(0));
    word_t data;
    core->readWord(regi / PRECISION, &data);
    core->push(data);
}

void do_load_nx(Core *core, const Insn *insn) { // pushram_nx
    word_t number = insn->getArg(0);
    REG_CODE regi = static_cast<REG_CODE>(insn->getArg(1));
    word_t data;
    core->readWord(regi / PRECISION + number, &data);
    core->push(data);
}

void do_load_xn(Core *core, const Insn *insn) { // pushram_xn
    REG_CODE regi = static_cast<REG_CODE>(insn->getArg(0));
    word_t number = insn->getArg(1);
    word_t data;
    core->readWord(regi / PRECISION + number, &data);
    core->push(data);
}
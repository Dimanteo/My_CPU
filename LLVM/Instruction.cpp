#include "Instruction.hpp"

bool Insn::isBranch() const {
    return m_isBranch;
}

int Insn::argc() const {
    return m_argc;
}

word_t Insn::getArg(int argi) const {
    return m_argv[argi];
}

size_t Insn::getSz() const {
    return sizeof(char) + m_argc * sizeof(word_t);
}

word_t Insn::fetchArg(const char *pc, int pos) {
    return ((word_t*)(pc + 1))[pos];
}

void Insn::decode(const char *pc) {
    m_code = static_cast<CMD_CODE>(*pc);
    switch (m_code) {
        case CMD_END: 
            m_isBranch = 0; 
            m_argc = 0; 
            execFName = "do_end";
            m_exec = nullptr; 
            break;
        case CMD_PUSH: 
            m_isBranch = 0; 
            m_argc = 1; 
            m_argv[0] = fetchArg(pc, 0);
            execFName = "do_push";
            m_exec = nullptr;
             break;
        case CMD_POP: 
            m_isBranch = false; 
            m_argc = 1;
            m_argv[0] = fetchArg(pc, 0);
            execFName = "do_pop"; 
            m_exec = nullptr; 
            break;
        case CMD_ADD: 
            m_isBranch = false; 
            m_argc = 0; 
            execFName = "do_add";
            m_exec = nullptr;
            break;
        case CMD_SUB: 
            m_isBranch = false;
            m_argc = 0;
            execFName = "do_sub";
            m_exec = nullptr;
            break;
        case CMD_MUL: 
            m_isBranch = false;
            m_argc = 0;  
            execFName = "do_mul"; 
            m_exec = nullptr; 
            break;
        case CMD_DIV: 
            m_isBranch = false;
            m_argc = 0;
            execFName = "do_div";
            m_exec = nullptr;
            break;
        case CMD_SQR: 
            m_isBranch = false;
            m_argc = 0;
            execFName = "do_sqr";
            m_exec = nullptr;
            break;
        case CMD_SIN: 
            m_isBranch = false;
            m_argc = 0;
            execFName = "do_sin";
            m_exec = nullptr;
            break;
        case CMD_COS: 
            m_isBranch = false; 
            m_argc = 0;
            execFName = "do_cos";
            m_exec = nullptr; 
            break;
        case CMD_OUT: 
            m_isBranch = false;
            m_argc = 0;
            execFName = "do_out"; 
            m_exec = nullptr;
            break;
        case CMD_IN: 
            m_isBranch = false;
            m_argc = 0;
            execFName = "do_in"; 
            m_exec = nullptr;
            break;
        case CMD_PUSHX: 
            m_isBranch = false; 
            m_argc = 1; 
            m_argv[0] = fetchArg(pc, 0); 
            execFName = "do_pushx";
            m_exec = nullptr; 
            break;
        case CMD_JUMP: 
            m_isBranch = true; 
            m_argc = 1; 
            m_argv[0] = fetchArg(pc, 0);
            execFName = "do_jump"; 
            m_exec = nullptr;
            break;
        case CMD_JUMPA: 
            m_isBranch = true;
            m_argc = 1;
            m_argv[0] = fetchArg(pc, 0);
            execFName = "do_jumpa";
            m_exec = nullptr;
            break;
        case CMD_JUMPAE: 
            m_isBranch = true; 
            m_argc = 1; 
            m_argv[0] = fetchArg(pc, 0); 
            execFName = "do_jumpae"; 
            m_exec = nullptr; 
            break;
        case CMD_JUMPB: 
            m_isBranch = true; 
            m_argc = 1;
            m_argv[0] = fetchArg(pc, 0); 
            execFName = "do_jumpb"; 
            m_exec = nullptr; 
            break;
        case CMD_JUMPBE: 
            m_isBranch = true;
            m_argc = 1;
            m_argv[0] = fetchArg(pc, 0);
            execFName = "do_jumpbe"; 
            m_exec = nullptr;
            break;
        case CMD_JUMPE: 
            m_isBranch = true;
            m_argc = 1; 
            m_argv[0] = fetchArg(pc, 0); 
            execFName = "do_jumpe"; 
            m_exec = nullptr; 
            break;
        case CMD_JUMPNE: 
            m_isBranch = true; 
            m_argc = 1; 
            m_argv[0] = fetchArg(pc, 0); 
            execFName = "do_jumpne"; 
            m_exec = nullptr; 
            break;
        case CMD_CALL: 
            m_isBranch = true; 
            m_argc = 1;
            m_argv[0] = fetchArg(pc, 0);
            execFName = "do_call"; 
            m_exec = nullptr; 
            break;
        case CMD_RET: 
            m_isBranch = true;
            m_argc = 0;
            execFName = "do_ret";
            m_exec = nullptr;
            break;
        case CMD_POPRAM_NX: 
            m_isBranch = false;
            m_argc = 2;
            m_argv[0] = fetchArg(pc, 0);
            m_argv[1] = fetchArg(pc, 1);
            execFName = "do_popramnx";
            m_exec = nullptr; 
            break;
        case CMD_POPRAM_XN: 
            m_isBranch = false;
            m_argc = 2;
            m_argv[0] = fetchArg(pc, 0); 
            m_argv[1] = fetchArg(pc, 1);
            execFName = "do_popramxn";
            m_exec = nullptr; 
            break;
        case CMD_POPRAM: 
            m_isBranch = false; 
            m_argc = 1; 
            m_argv[0] = fetchArg(pc, 0); 
            execFName = "do_popram"; 
            m_exec = nullptr; 
            break;
        case CMD_PUSHRAM: 
            m_isBranch = false;
            m_argc = 1;
            m_argv[0] = fetchArg(pc, 0);
            execFName = "do_pushram"; 
            m_exec = nullptr;
            break;
        case CMD_POPRAM_X: 
            m_isBranch = false;
            m_argc = 1;
            m_argv[0] = fetchArg(pc, 0);
            execFName = "do_popramx"; 
            m_exec = nullptr; 
            break;
        case CMD_PUSHRAM_X: 
            m_isBranch = false;
            m_argc = 1;
            m_argv[0] = fetchArg(pc, 0); 
            execFName = "do_pushramx";
            m_exec = nullptr; 
            break;
        case CMD_PUSHRAM_NX: 
            m_isBranch = false;
            m_argc = 2; 
            m_argv[0] = fetchArg(pc, 0); 
            m_argv[1] = fetchArg(pc, 1);
            execFName = "do_pushramnx"; 
            m_exec = nullptr; 
            break;
        case CMD_PUSHRAM_XN: 
            m_isBranch = false; 
            m_argc = 2; 
            m_argv[0] = fetchArg(pc, 0); 
            m_argv[1] = fetchArg(pc, 1); 
            execFName = "do_pushramxn";
            m_exec = nullptr; 
            break;
        case CMD_POWER: 
            m_isBranch = false; 
            m_argc = 0; 
            execFName = "do_power"; 
            m_exec = nullptr;
            break;
    }
}
#define DEF_CMD(CMD_name, token, scanf_sample, number_of_args, instructions, disasm_print) \
    case CMD_##CMD_name: \
        m_isBranch = ; \
        m_argc = number_of_args; \
        m_argv[] = ;\
        execFName = ;\
        m_exec = nullptr;\
        break;

    #include "../commands.h"

#undef DEF_CMD

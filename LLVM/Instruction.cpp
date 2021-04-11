#include "Instruction.hpp"
#include "exec.hpp"
#include "generate.hpp"

bool Insn::isBranch() const { return m_isBranch; }

bool Insn::isTerm() const { return m_isTerminator; }

int Insn::argc() const { return m_argc; }

word_t Insn::getArg(int argi) const { return m_argv[argi]; }

size_t Insn::getSz() const { return sizeof(char) + m_argc * sizeof(word_t); }

CMD_CODE Insn::getCode() const { return m_code; }

void Insn::exec(Core *core) const { m_exec(core, this); }

void Insn::generateIR(llvm::IRBuilder<> *builder, Core &core) {
    // insert IR callback
    gen_callback(builder, core, *this);
    // insert instruction code
    if (m_genIR != nullptr)
        m_genIR(builder, core, *this);
    else
        gen_default(builder, core, *this);
}

std::string Insn::getName() const { return execFName; }

word_t Insn::fetchArg(const char *pc, int pos) {
    return ((word_t *)(pc + 1))[pos];
}

void *Insn::lazyFunctionCreator(const std::string fname) {
    return reinterpret_cast<void *>(functionCreatorMap[fname]);
}

void Insn::decode(const char *pc) {
    m_code = static_cast<CMD_CODE>(*pc);
    m_isTerminator = false;
    switch (m_code) {
    case CMD_END:
        m_isBranch = false;
        m_isTerminator = true;
        m_argc = 0;
        execFName = "do_end";
        m_exec = do_end;
        m_genIR = gen_end;
        break;
    case CMD_PUSH:
        m_isBranch = false;
        m_argc = 1;
        m_argv[0] = fetchArg(pc, 0);
        execFName = "do_push";
        m_exec = do_push;
        m_genIR = nullptr;
        break;
    case CMD_POP:
        m_isBranch = false;
        m_argc = 1;
        m_argv[0] = fetchArg(pc, 0);
        execFName = "do_pop";
        m_exec = do_pop;
        m_genIR = nullptr;
        break;
    case CMD_ADD:
        m_isBranch = false;
        m_argc = 0;
        execFName = "do_add";
        m_exec = do_add;
        m_genIR = nullptr;
        break;
    case CMD_SUB:
        m_isBranch = false;
        m_argc = 0;
        execFName = "do_sub";
        m_exec = do_sub;
        m_genIR = nullptr;
        break;
    case CMD_MUL:
        m_isBranch = false;
        m_argc = 0;
        execFName = "do_mul";
        m_exec = do_mul;
        m_genIR = nullptr;
        break;
    case CMD_DIV:
        m_isBranch = false;
        m_argc = 0;
        execFName = "do_div";
        m_exec = do_div;
        m_genIR = nullptr;
        break;
    case CMD_SQR:
        m_isBranch = false;
        m_argc = 0;
        execFName = "do_sqr";
        m_exec = do_sqr;
        break;
    case CMD_SIN:
        m_isBranch = false;
        m_argc = 0;
        execFName = "do_sin";
        m_exec = do_sin;
        m_genIR = nullptr;
        break;
    case CMD_COS:
        m_isBranch = false;
        m_argc = 0;
        execFName = "do_cos";
        m_exec = do_cos;
        m_genIR = nullptr;
        break;
    case CMD_OUT:
        m_isBranch = false;
        m_argc = 0;
        execFName = "do_out";
        m_exec = do_out;
        m_genIR = nullptr;
        break;
    case CMD_IN:
        m_isBranch = false;
        m_argc = 0;
        execFName = "do_in";
        m_exec = do_in;
        m_genIR = nullptr;
        break;
    case CMD_PUSHX:
        m_isBranch = false;
        m_argc = 1;
        m_argv[0] = fetchArg(pc, 0);
        execFName = "do_pushx";
        m_exec = do_pushx;
        m_genIR = nullptr;
        break;
    case CMD_JUMP:
        m_isBranch = true;
        m_isTerminator = true;
        m_argc = 1;
        m_argv[0] = fetchArg(pc, 0);
        execFName = "do_jump";
        m_exec = nullptr;
        m_genIR = gen_jump;
        break;
    case CMD_JUMPA:
        m_isBranch = true;
        m_isTerminator = true;
        m_argc = 1;
        m_argv[0] = fetchArg(pc, 0);
        execFName = "do_jumpa";
        m_exec = nullptr;
        m_genIR = gen_jumpa;
        break;
    case CMD_JUMPAE:
        m_isBranch = true;
        m_isTerminator = true;
        m_argc = 1;
        m_argv[0] = fetchArg(pc, 0);
        execFName = "do_jumpae";
        m_exec = nullptr;
        m_genIR = gen_jumpae;
        break;
    case CMD_JUMPB:
        m_isBranch = true;
        m_isTerminator = true;
        m_argc = 1;
        m_argv[0] = fetchArg(pc, 0);
        execFName = "do_jumpb";
        m_exec = nullptr;
        m_genIR = gen_jumpb;
        break;
    case CMD_JUMPBE:
        m_isBranch = true;
        m_isTerminator = true;
        m_argc = 1;
        m_argv[0] = fetchArg(pc, 0);
        execFName = "do_jumpbe";
        m_exec = nullptr;
        m_genIR = gen_jumpbe;
        break;
    case CMD_JUMPE:
        m_isBranch = true;
        m_isTerminator = true;
        m_argc = 1;
        m_argv[0] = fetchArg(pc, 0);
        execFName = "do_jumpe";
        m_exec = nullptr;
        m_genIR = gen_jumpeq;
        break;
    case CMD_JUMPNE:
        m_isBranch = true;
        m_isTerminator = true;
        m_argc = 1;
        m_argv[0] = fetchArg(pc, 0);
        execFName = "do_jumpne";
        m_exec = nullptr;
        m_genIR = gen_jumpne;
        break;
    case CMD_CALL:
        m_isBranch = true;
        m_argc = 1;
        m_argv[0] = fetchArg(pc, 0);
        execFName = "do_call";
        m_exec = nullptr;
        m_genIR = gen_call;
        break;
    case CMD_RET:
        m_isBranch = false;
        m_isTerminator = true;
        m_argc = 0;
        execFName = "do_ret";
        m_exec = nullptr;
        m_genIR = gen_ret;
        break;
    case CMD_POPRAM_NX:
        m_isBranch = false;
        m_argc = 2;
        m_argv[0] = fetchArg(pc, 0);
        m_argv[1] = fetchArg(pc, 1);
        execFName = "do_popramnx";
        m_exec = do_store_nx;
        m_genIR = nullptr;
        break;
    case CMD_POPRAM_XN:
        m_isBranch = false;
        m_argc = 2;
        m_argv[0] = fetchArg(pc, 0);
        m_argv[1] = fetchArg(pc, 1);
        execFName = "do_popramxn";
        m_exec = do_store_xn;
        m_genIR = nullptr;
        break;
    case CMD_POPRAM:
        m_isBranch = false;
        m_argc = 1;
        m_argv[0] = fetchArg(pc, 0);
        execFName = "do_popram";
        m_exec = do_store;
        m_genIR = nullptr;
        break;
    case CMD_PUSHRAM:
        m_isBranch = false;
        m_argc = 1;
        m_argv[0] = fetchArg(pc, 0);
        execFName = "do_pushram";
        m_exec = do_load;
        m_genIR = nullptr;
        break;
    case CMD_POPRAM_X:
        m_isBranch = false;
        m_argc = 1;
        m_argv[0] = fetchArg(pc, 0);
        execFName = "do_popramx";
        m_exec = do_store_x;
        m_genIR = nullptr;
        break;
    case CMD_PUSHRAM_X:
        m_isBranch = false;
        m_argc = 1;
        m_argv[0] = fetchArg(pc, 0);
        execFName = "do_pushramx";
        m_exec = do_load_x;
        m_genIR = nullptr;
        break;
    case CMD_PUSHRAM_NX:
        m_isBranch = false;
        m_argc = 2;
        m_argv[0] = fetchArg(pc, 0);
        m_argv[1] = fetchArg(pc, 1);
        execFName = "do_pushramnx";
        m_exec = do_load_nx;
        m_genIR = nullptr;
        break;
    case CMD_PUSHRAM_XN:
        m_isBranch = false;
        m_argc = 2;
        m_argv[0] = fetchArg(pc, 0);
        m_argv[1] = fetchArg(pc, 1);
        execFName = "do_pushramxn";
        m_exec = do_load_xn;
        m_genIR = nullptr;
        break;
    case CMD_POWER:
        m_isBranch = false;
        m_argc = 0;
        execFName = "do_power";
        m_exec = do_power;
        m_genIR = nullptr;
        break;
    case ISA_POWER:
    default:
        m_isBranch = false;
        m_argc = 0;
        execFName = nullptr;
        m_exec = nullptr;
        m_genIR = nullptr;
        break;
    }
    if (functionCreatorMap.find(execFName) == functionCreatorMap.end())
        functionCreatorMap.insert(
            {execFName, reinterpret_cast<void *>(m_exec)});
}

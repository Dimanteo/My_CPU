#include "LLVM_IR.hpp"
#include "Core.hpp"
#include "Instruction.hpp"

int main(int argc, char* argv[])
{
    if (argc < 2)
    {
        fprintf(stderr, "Usage: %s <binary> [<file>]\n", argv[0]);
        return EXIT_FAILURE;
    }

    char* bin_file = argv[1];
    std::string ir_file("llvmir.ll");
    if (argc > 2)
        ir_file = argv[2];

    size_t code_size = 0;
    char *binary = read_file_to_buffer_alloc(argv[1], "rb", &code_size);
    int code_offs = check_binary_source(binary);

    Core core;
    core.run(binary, code_offs, code_size);

    free(binary);
}

void do_END(CPU *cpu, const char* pc)
{
    cpu->run = false;
}

void do_PUSH(CPU *cpu, const char* pc) 
{
    stack_push(&cpu->stack, *(int*)(pc + 1));
}

void do_POP(CPU *cpu, const char* pc) 
{
    int reg_i = *(int*)(pc + 1);
    cpu->reg[reg_i] = stack_pop(&cpu->stack);
}

void do_ADD(CPU *cpu, const char* pc)
{
    int arg1 = stack_pop(&cpu->stack);
    int arg2 = stack_pop(&cpu->stack);
    stack_push(&cpu->stack, arg1 + arg2);
}

void do_SUB(CPU *cpu, const char* pc)
{
    int rhs = stack_pop(&cpu->stack);
    int lhs = stack_pop(&cpu->stack);
    stack_push(&cpu->stack, lhs - rhs);
}

void do_MUL(CPU *cpu, const char* pc)
{
    int arg1 = stack_pop(&cpu->stack);
    int arg2 = stack_pop(&cpu->stack);
    stack_push(&cpu->stack, arg1 * arg2 / PRECISION);
}

void do_DIV(CPU *cpu, const char* pc)
{
    float rhs = static_cast<float>(stack_pop(&cpu->stack));
    float lhs = static_cast<float>(stack_pop(&cpu->stack));
    stack_push(&cpu->stack, static_cast<int>(lhs / rhs * PRECISION));
}

void do_SQR(CPU *cpu, const char* pc)
{
    double val = static_cast<double>(stack_pop(&cpu->stack));
    int res = static_cast<int>(sqrt(val / PRECISION) * PRECISION);
    stack_push(&cpu->stack, res);
}
void do_SIN(CPU *cpu, const char* pc)
{
    double val = static_cast<double>(stack_pop(&cpu->stack));
    int res = static_cast<int>(sin(val / PRECISION) * PRECISION);
    stack_push(&cpu->stack, res);
}
void do_COS(CPU *cpu, const char* pc)
{
    double val = static_cast<double>(stack_pop(&cpu->stack));
    int res = static_cast<int>(cos(val / PRECISION) * PRECISION);
    stack_push(&cpu->stack, res);
}
void do_OUT(CPU *cpu, const char* pc)
{
float val = static_cast<float>(stack_pop(&cpu->stack)) / PRECISION;
    printf("%.3g\n", val);
}
void do_IN(CPU *cpu, const char* pc)
{
    float val = 0;
    scanf("%f", &val);
    stack_push(&cpu->stack, static_cast<int>(val * PRECISION));
}

void do_PUSHX(CPU *cpu, const char* pc) 
{
    int reg_i = *(int*)(pc + 1);
    stack_push(&cpu->stack, cpu->reg[reg_i]);
}

void do_JUMP(CPU *cpu, const char* pc) {}
void do_JUMPA(CPU *cpu, const char* pc) {}
void do_JUMPAE(CPU *cpu, const char* pc) {}
void do_JUMPB(CPU *cpu, const char* pc) {}
void do_JUMPBE(CPU *cpu, const char* pc) {}
void do_JUMPE(CPU *cpu, const char* pc) {}
void do_JUMPNE(CPU *cpu, const char* pc) {}
void do_CALL(CPU *cpu, const char* pc) {}
void do_RET(CPU *cpu, const char* pc) {}
void do_POPRAM_NX(CPU *cpu, const char* pc) {}
void do_POPRAM_XN(CPU *cpu, const char* pc) {}
void do_POPRAM(CPU *cpu, const char* pc) {}
void do_PUSHRAM(CPU *cpu, const char* pc) {}
void do_POPRAM_X(CPU *cpu, const char* pc) {}
void do_PUSHRAM_X(CPU *cpu, const char* pc) {}
void do_PUSHRAM_NX(CPU *cpu, const char* pc) {}
void do_PUSHRAM_XN(CPU *cpu, const char* pc) {}

void do_POWER(CPU *cpu, const char* pc)
{
    double power = static_cast<double>(stack_pop(&cpu->stack)) / PRECISION;
    double base  = static_cast<double>(stack_pop(&cpu->stack)) / PRECISION;
    int res = static_cast<int>(pow(base, power) * PRECISION);
    stack_push(&cpu->stack, res);
}

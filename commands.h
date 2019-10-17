
//DSL
#define cPOP stack_pop(&cpu.stack)
#define cPUSH(x) stack_push(&cpu.stack, x)
#define cREG(x) cpu.reg[x]
#define cPRINT(x) fprintf(cpu_out, "%g\n", x)

    DEF_CMD(END, 0, 0, )
    DEF_CMD(PUSH, 1, 1, cPUSH((double)arg_v[0] / 1000))
    DEF_CMD(POP, 2, 1, cREG(arg_v[0]) = cPOP)
    DEF_CMD(ADD, 3, 0, cPUSH(cPOP + cPOP))
    DEF_CMD(SUB, 4, 0, cPUSH(-cPOP + cPOP))
    DEF_CMD(MUL, 5, 0, cPUSH(cPOP * cPOP))
    DEF_CMD(DIV, 6, 0, cPUSH(1 / cPOP * cPOP))
    DEF_CMD(SQR, 7, 0, cPUSH(sqrt(cPOP)))
    DEF_CMD(SIN, 8, 0, cPUSH(sin(cPOP)))
    DEF_CMD(COS, 9, 0, cPUSH(cos(cPOP)))
    DEF_CMD(OUT, 10, 0, cPRINT(cPOP))

#undef cPOP
#undef cPUSH
#undef cREG
#undef cPRINT
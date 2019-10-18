
//DSL
#define cPOP stack_pop(&cpu.stack)
#define cPUSH(x) stack_push(&cpu.stack, x)
#define cREG(x) cpu.reg[x]
#define cPRINT(x) fprintf(cpu_out, "%g\n", x)
#define SCANsample(format, arguments) && (sscanf(data[pc].begin + symb_passed, format, arguments))
#define regTOKENS "%*[ ]%[xabcd]"

#define $ , //need to avoid problems with comas in scanf with 2 or more arguments

/* DEF_CMD(CMD_name, token, scanf_sample,  code, number_of_args, instructions)*/
    DEF_CMD(END, end, /*scanf_sample*/, 0, 0, /*instructios*/)
    DEF_CMD(PUSH, push, SCANsample("%lf", darg), 1, 1, cPUSH((double)arg_v[0] / 1000)) //push to stack
    DEF_CMD(POP, pop, SCANsample( regTOKENS , sarg), 2, 1, cREG(arg_v[0]) = cPOP)
    DEF_CMD(ADD, add, /*scanf_sample*/, 3, 0, cPUSH(cPOP + cPOP))
    DEF_CMD(SUB, sub, /*scanf_sample*/, 4, 0, cPUSH(-cPOP + cPOP))
    DEF_CMD(MUL, mul, /*scanf_sample*/, 5, 0, cPUSH(cPOP * cPOP))
    DEF_CMD(DIV, div, /*scanf_sample*/, 6, 0, cPUSH(1 / cPOP * cPOP))
    DEF_CMD(SQR, sqr, /*scanf_sample*/, 7, 0, cPUSH(sqrt(cPOP)))
    DEF_CMD(SIN, sin, /*scanf_sample*/, 8, 0, cPUSH(sin(cPOP)))
    DEF_CMD(COS, cos, /*scanf_sample*/, 9, 0, cPUSH(cos(cPOP)))
    DEF_CMD(OUT, out, /*scanf_sample*/, 10, 0, cPRINT(cPOP)) //print value from stack to screen
    DEF_CMD(PUSHX, push, SCANsample( regTOKENS , sarg), 11, 1, cPUSH(cREG(arg_v[0]))) //Push to register

#undef $

#undef regTOKENS
#undef cPOP
#undef cPUSH
#undef cREG
#undef cPRINT
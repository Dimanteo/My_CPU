#define PRECISION 1000 //const

//DSL
#define cPOP stack_pop(&cpu.stack)
#define cPUSH(x) stack_push(&cpu.stack, x)
#define cREG(x) cpu.reg[x]
#define cPRINT(x) fprintf(cpu_out, "%g\n", (double)x / PRECISION)
#define SCANsample(format, arguments) && (sscanf(data[pc].begin + symb_passed, format, arguments))
#define regTOKENS " %[xabcd]"
#define getARG " %g"$ (double)*(int*)(pc + 1) / PRECISION
#define getARG_REG " %s"$ translate_code(*(int*)(pc + 1))
#define $ , //need to avoid problems with comas in functions with 2 or more arguments

/* DEF_CMD(CMD_name, token, scanf_sample,  code, number_of_args, instructions, disasm_print)*/
    DEF_CMD(END, end, /*scanf_sample*/, 0, 0, /*instructios*/, /*disasm_print*/)
    DEF_CMD(PUSH, push, SCANsample( " %lf" , darg), 1, 1, cPUSH(arg_v[0]), getARG) //push to stack
    DEF_CMD(POP, pop, SCANsample( regTOKENS , sarg), 2, 1, cREG(arg_v[0]) = cPOP, getARG_REG) //put in register
    DEF_CMD(ADD, add, /*scanf_sample*/, 3, 0, cPUSH(cPOP + cPOP), /*disasm_print*/)
    DEF_CMD(SUB, sub, /*scanf_sample*/, 4, 0, cPUSH(-cPOP + cPOP), /*disasm_print*/)
    DEF_CMD(MUL, mul, /*scanf_sample*/, 5, 0, cPUSH(cPOP * cPOP / PRECISION), /*disasm_print*/)
    DEF_CMD(DIV, div, /*scanf_sample*/, 6, 0, cPUSH((int)(1 / (double) cPOP * cPOP * PRECISION)), /*disasm_print*/)
    DEF_CMD(SQR, sqr, /*scanf_sample*/, 7, 0, cPUSH((int)(sqrt((double)cPOP / PRECISION) * PRECISION)), /*disasm_print*/)
    DEF_CMD(SIN, sin, /*scanf_sample*/, 8, 0, cPUSH((int)(sin((double)cPOP / PRECISION) * PRECISION)), /*disasm_print*/)
    DEF_CMD(COS, cos, /*scanf_sample*/, 9, 0, cPUSH((int)(cos((double)cPOP / PRECISION) * PRECISION)), /*disasm_print*/)
    DEF_CMD(OUT, out, /*scanf_sample*/, 10, 0, cPRINT(cPOP), /*disasm_print*/) //print value from stack to screen
    DEF_CMD(PUSHX, push, SCANsample( regTOKENS , sarg), 11, 1, cPUSH(cREG(arg_v[0])), getARG_REG) //Push from register
    DEF_CMD(JUMP, jmp, SCANsample(" %s", sarg), 12, 1, pc = bin + arg_v[0]; break; , " %d"$ *(int*)(pc + 1))
    DEF_CMD(JUMPA, ja, SCANsample(" %s", sarg), 13, 1, if(cPOP > cPOP) {pc = bin + arg_v[0]; break;} , " %d"$ *(int*)(pc + 1))
    DEF_CMD(JUMPAE, jae, SCANsample(" %s", sarg), 14, 1, if(cPOP >= cPOP) {pc = bin + arg_v[0]; break;} , " %d"$ *(int*)(pc + 1))
    DEF_CMD(JUMPB, jb, SCANsample(" %s", sarg), 15, 1, if(cPOP < cPOP) {pc = bin + arg_v[0]; break;} , " %d"$ *(int*)(pc + 1))
    DEF_CMD(JUMPBE, jbe, SCANsample(" %s", sarg), 16, 1, if(cPOP <= cPOP) {pc = bin + arg_v[0]; break;} , " %d"$ *(int*)(pc + 1))
    DEF_CMD(JUMPE, je, SCANsample(" %s", sarg), 17, 1, if(cPOP == cPOP) {pc = bin + arg_v[0]; break;} , " %d"$ *(int*)(pc + 1))
    DEF_CMD(JUMPNE, jne, SCANsample(" %s", sarg), 18, 1, if(cPOP != cPOP) {pc = bin + arg_v[0]; break;} , " %d"$ *(int*)(pc + 1))
    DEF_CMD(CALL, call, SCANsample(" %s", sarg), 19, 1, cPUSH(pc + 1 + sizeof(int) - bin); pc = bin + arg_v[0]; break; , " %d"$ *(int*)(pc + 1))
    DEF_CMD(RET, ret, /*scanf_sample*/, 20, 0, pc = bin + cPOP; break; , /*disasm_print*/)

#undef $
#undef getARG
#undef getARG_REG
#undef regTOKENS
#undef cPOP
#undef cPUSH
#undef cREG
#undef cPRINT
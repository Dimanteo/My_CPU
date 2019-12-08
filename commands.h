#define PRECISION 1000

//DSL
#define cPOP stack_pop(&cpu.stack)
#define cPUSH(x) stack_push(&cpu.stack, x)
#define cREG(x) cpu.reg[x]
#define cPRINT(x) fprintf(cpu_out, "%g\n", (double)x / PRECISION)
#define SCANsample(format, arguments) && sscanf(data[pc].begin + symb_passed, format, arguments)
#define regTOKENS " %[xabcd]"
#define getARG " %g"$ (double)*(int*)(pc + 1) / PRECISION
#define getARG_REG " %s"$ translate_code(*(int*)(pc + 1))
#define $ , //need to avoid problems with comas in functions with 2 or more arguments

/* DEF_CMD(CMD_name, token, scanf_sample, number_of_args, instructions, disasm_print)*/
    DEF_CMD(END, end, /*scanf_sample*/, 0, /*instructios*/, /*disasm_print*/)
    DEF_CMD(PUSH, push, SCANsample( " %lf" , darg), 1, cPUSH(arg_v[0]), getARG) //push to stack
    DEF_CMD(POP, pop, SCANsample( regTOKENS , sarg), 1, cREG(arg_v[0]) = cPOP, getARG_REG) //put in register
    DEF_CMD(ADD, add, /*scanf_sample*/, 0, cPUSH(cPOP + cPOP), /*disasm_print*/)
    DEF_CMD(SUB, sub, /*scanf_sample*/, 0, cPUSH(-cPOP + cPOP), /*disasm_print*/)
    DEF_CMD(MUL, mul, /*scanf_sample*/, 0, cPUSH(cPOP * cPOP / PRECISION), /*disasm_print*/)
    DEF_CMD(DIV, div, /*scanf_sample*/, 0, cPUSH((int)(1 / (double) cPOP * cPOP * PRECISION)), /*disasm_print*/)
    DEF_CMD(SQR, sqr, /*scanf_sample*/, 0, cPUSH((int)(sqrt((double)cPOP / PRECISION) * PRECISION)), /*disasm_print*/)
    DEF_CMD(SIN, sin, /*scanf_sample*/, 0, cPUSH((int)(sin((double)cPOP / PRECISION) * PRECISION)), /*disasm_print*/)
    DEF_CMD(COS, cos, /*scanf_sample*/, 0, cPUSH((int)(cos((double)cPOP / PRECISION) * PRECISION)), /*disasm_print*/)
    DEF_CMD(OUT, out, /*scanf_sample*/, 0, cPRINT(cPOP), /*disasm_print*/) //print value from stack to screen
    DEF_CMD(PUSHX, push, SCANsample( regTOKENS , sarg), 1, cPUSH(cREG(arg_v[0])), getARG_REG) //Push from register
    DEF_CMD(JUMP, jmp, SCANsample(" %s", sarg), 1, pc = bin + arg_v[0]; break; , " %d"$ *(int*)(pc + 1))
    DEF_CMD(JUMPA, ja, SCANsample(" %s", sarg), 1, if(cPOP > cPOP) {pc = bin + arg_v[0]; break;} , " %d"$ *(int*)(pc + 1))
    DEF_CMD(JUMPAE, jae, SCANsample(" %s", sarg), 1, if(cPOP >= cPOP) {pc = bin + arg_v[0]; break;} , " %d"$ *(int*)(pc + 1))
    DEF_CMD(JUMPB, jb, SCANsample(" %s", sarg), 1, if(cPOP < cPOP) {pc = bin + arg_v[0]; break;} , " %d"$ *(int*)(pc + 1))
    DEF_CMD(JUMPBE, jbe, SCANsample(" %s", sarg), 1, if(cPOP <= cPOP) {pc = bin + arg_v[0]; break;} , " %d"$ *(int*)(pc + 1))
    DEF_CMD(JUMPE, je, SCANsample(" %s", sarg), 1, if(cPOP == cPOP) {pc = bin + arg_v[0]; break;} , " %d"$ *(int*)(pc + 1))
    DEF_CMD(JUMPNE, jne, SCANsample(" %s", sarg), 1, if(cPOP != cPOP) {pc = bin + arg_v[0]; break;} , " %d"$ *(int*)(pc + 1))
    DEF_CMD(CALL, call, SCANsample(" %s", sarg), 1, cPUSH(pc + 1 + sizeof(int) - bin); pc = bin + arg_v[0]; break; , " %d"$ *(int*)(pc + 1))
    DEF_CMD(RET, ret, /*scanf_sample*/, 0, pc = bin + cPOP; break; , /*disasm_print*/)
    DEF_CMD(PUSHRAM_NX, push, SCANsample(" [%[^]]]", sarg), 2, cpu.RAM[arg_v[0] + cREG(arg_v[1]) / PRECISION] = cPOP, " [%d + %s]" $ *(int*)(pc + 1) $ translate_code(*(int*)(pc + 1) + 1))
    DEF_CMD(PUSHRAM_XN, push, SCANsample(" [%[^]]]", sarg), 2, cpu.RAM[arg_v[1] + cREG(arg_v[0]) / PRECISION] = cPOP, " [%s + %d]" $ translate_code(*(int*)(pc + 1)) $ *((int*)(pc + 1) + 1))
    DEF_CMD(PUSHRAM, push, SCANsample(" [%[^]]]", sarg), 1, cpu.RAM[arg_v[0]] = cPOP, " [%d]" $ *(int*)(pc + 1))
    DEF_CMD(POPRAM, pop, SCANsample(" [%[^]]]", sarg), 1, cPUSH(cpu.RAM[arg_v[0]]), " [%d]" $ *(int*)(pc + 1))
    DEF_CMD(PUSHRAM_X, push, SCANsample(" [%[^]]]", sarg), 1, cpu.RAM[cREG(arg_v[0]) / PRECISION] = cPOP, getARG_REG)
    DEF_CMD(POPRAM_X, pop, SCANsample(" [%[^]]]", sarg), 1, cPUSH(cpu.RAM[cREG(arg_v[0]) / PRECISION]), getARG_REG)
    DEF_CMD(POPRAM_NX, pop, SCANsample(" [%[^]]]", sarg), 2, cPUSH(cpu.RAM[arg_v[0] + cREG(arg_v[1]) / PRECISION]), " [%d + %s]" $ *(int*)(pc + 1) $ translate_code(*(int*)(pc + 1) + 1))
    DEF_CMD(POPRAM_XN, pop, SCANsample(" [%[^]]]", sarg), 2, cPUSH(cpu.RAM[arg_v[1] + cREG(arg_v[0]) / PRECISION]), " [%s + %d]" $ translate_code(*(int*)(pc + 1)) $ *((int*)(pc + 1) + 1))

#undef $
#undef getARG
#undef getARG_REG
#undef regTOKENS
#undef cPOP
#undef cPUSH
#undef cREG
#undef cPRINT
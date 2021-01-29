#include "cpuStruct.h"


bool cpu_verify (CPU* cpu, const char filename[], const char function[], int line) {
#ifndef NDEBUG

    if(!soft_assert(cpu->cpu_canary1 == CANARY_VALUE, filename, function, line)) {
        cpu_dump(cpu, "cpu_canary1 has changed", ERR_STATE, filename, function, line);
        return false;
    }
    if(!soft_assert(cpu->cpu_canary2 == CANARY_VALUE, filename, function, line)) {
        cpu_dump(cpu, "cpu_canary2 has changed", ERR_STATE, filename, function, line);
        return false;
    }
    if(!stack_OK(&cpu->stack, filename, function, line)) {
        cpu_dump(cpu, "cpu.stack error", ERR_STATE, filename, function, line);
        return false;
    }

#ifdef OK_DUMP
    cpu_dump(cpu, "Calm down. It's ok", OK_STATE, filename, function, line);
#endif

#endif
    return true;
}

void cpu_init(CPU* cpu) {
    cpu->cpu_canary1 = CANARY_VALUE;
    cpu->stack = (Stack_t){"CPU_stack"};

    stack_init(&cpu->stack);//default stack size = 10

    cpu->reg[AX] = POISON;
    cpu->reg[BX] = POISON;
    cpu->reg[CX] = POISON;
    cpu->reg[DX] = POISON;

    cpu->run = false;

    cpu->cpu_canary2 = CANARY_VALUE;

    cpu_verify(cpu, __FILE__, __PRETTY_FUNCTION__, __LINE__);
}

void cpu_destruct(CPU *cpu) {
    cpu_verify(cpu,  __FILE__, __PRETTY_FUNCTION__, __LINE__);

    cpu->cpu_canary1 = 0;

    stack_destruct(&cpu->stack);
    cpu->reg[AX] = 0;
    cpu->reg[BX] = 0;
    cpu->reg[CX] = 0;
    cpu->reg[DX] = 0;

    cpu->cpu_canary2 = 0;
}

void cpu_dump(CPU *cpu, const char reason[], const char state[], const char filename[], const char func[], int line) {

    FILE* log = fopen(CPU_LOG_NAME, "ab");
    assert(log);

    if(cpu == nullptr) {
        fprintf(log, "Dump(%s) From %s; %s (%d)\n\tCPU [NULL pointer] (%s)\n", reason, filename, func, line, state);
        fclose(log);
        return;
    }

    fprintf(log, "\nCPU Dump(%s) From %s; %s (%d)\n"
                 "\tCPU %s [%p] (%s)\n"
                 "\tcpu_canary1 = " CANARY_PRINT ";\n"
                 "\tregister ax = " ELEMENT_PRINT "; %s\n"
                 "\tregister bx = " ELEMENT_PRINT "; %s\n"
                 "\tregister cx = " ELEMENT_PRINT "; %s\n"
                 "\tregister dx = " ELEMENT_PRINT "; %s\n"
                 "\tStack_t stack[%p] -> %s;\n"
                 "\tcpu_canary2 = " CANARY_PRINT ";\n"
                 "CPU.stack dump \n{",
                 reason, filename, func, line, cpu->tag, cpu, state, cpu->cpu_canary1,
                 cpu->reg[AX], cpu->reg[AX] == POISON ? "(POISON?)" : "",
                 cpu->reg[BX], cpu->reg[BX] == POISON ? "(POISON?)" : "",
                 cpu->reg[CX], cpu->reg[CX] == POISON ? "(POISON?)" : "",
                 cpu->reg[DX], cpu->reg[DX] == POISON ? "(POISON?)" : "",
                 &cpu->stack, cpu->stack.tag, cpu->cpu_canary2);
    fclose(log);

    stack_reopen_log(&cpu->stack, CPU_LOG_NAME);
    stack_dump(&cpu->stack, "call by CPU_dump", (strcmp(state, OK_STATE) == 0) ? PARENT_OK : PARENT_ERR, filename, func, line);
    stack_reopen_log(&cpu->stack, STACK_LOG_NAME);

    log = fopen(CPU_LOG_NAME, "ab");
    assert(log);
    fprintf(log, "RAM {\n");
    for (int i = 0; i < RAM_SIZE; ++i) {
        fprintf(log, "\t[%d]: %d\n", i, cpu->RAM[i]);
    }
    fprintf(log, "}\n} end of CPU Dump(%s)\n", cpu->tag);

    fclose(log);
}

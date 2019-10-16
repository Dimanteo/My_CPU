#include "cassert"
#include "My_Headers/protected_stack.h"
#include "My_Headers/txt_files.h"
#include "linker.h"

struct CPU {
    const char tag[MAX_NAME_LENGTH];
    canary_t cpu_canary1;
    Stack_t stack;
    int ax;
    int bx;
    int cx;
    int dx;
    canary_t cpu_canary2;
};

void cpu_init(CPU* cpu);

void cpu_destruct(CPU* cpu);

bool cpu_verify(CPU* cpu);

void cpu_dump(CPU* cpu, const char reason[], const char state[], const char filename[], const char func[], int line);

int main() {
    CPU cpu = {"cpu"};
    cpu_init(&cpu);
    size_t size_bin = 0;
    char* bin = read_file_to_buffer_alloc(BIN_FILE_NAME, "rb", &size_bin);
    assert(bin);

    for (char* pc = bin; *pc < 0; ++pc) {

    }

    cpu_destruct(&cpu);
    free(bin);
}

bool cpu_verify (CPU* cpu) {
#ifndef NDEBUG

    if(!soft_assert(cpu->cpu_canary1 == CANARY_VALUE, __FILE__, __PRETTY_FUNCTION__, __LINE__)) {
        cpu_dump(cpu, "cpu_canary1 has changed", ERR_STATE, __FILE__, __PRETTY_FUNCTION__, __LINE__);
    }
    if(soft_assert(cpu->cpu_canary2 == CANARY_VALUE, __FILE__, __PRETTY_FUNCTION__, __LINE__)) {
        cpu_dump(cpu, "cpu_canary2 has changed", ERR_STATE, __FILE__, __PRETTY_FUNCTION__, __LINE__);
    }
    stack_OK(&cpu->stack, __FILE__, __PRETTY_FUNCTION__, __LINE__);

#endif
}

void cpu_init(CPU* cpu) {
    cpu->cpu_canary1 = CANARY_VALUE;
    cpu->stack = {"CPU_stack"};
    stack_init(&cpu->stack);//default stack size = 10
    cpu->ax = POISON;
    cpu->bx = POISON;
    cpu->cx = POISON;
    cpu->dx = POISON;
    cpu->cpu_canary2 = CANARY_VALUE;
    cpu_verify(cpu);
}

void cpu_destruct(CPU *cpu) {
    cpu_verify(cpu);
    cpu->cpu_canary1 = 0;
    stack_destruct(&cpu->stack);
    cpu->ax = 0;
    cpu->bx = 0;
    cpu->cx = 0;
    cpu->dx = 0;
    cpu->cpu_canary2 = 0;
}

void cpu_dump(CPU *cpu, const char reason[], const char state[], const char filename[], const char func[], int line) {
    FILE* log = fopen(LOG_NAME, LOG_OPENED ? "a" : "w");
    LOG_OPENED = true;
    assert(log);

    if(cpu == nullptr) {
        fprintf(log, "Dump(%s) From %s; %s (%d)\n\tCPU [NULL pointer] (%s)\n", reason, filename, func, line, state);
        return;
    }

    fprintf(log, "\nCPU Dump(%s) From %s; %s (%d)\n"
                 "\tCPU [%p] (%s)\n"
                 "\tcpu_canary1 = " CANARY_PRINT ";\n"
                 "\tregister ax = %d; %s"
                 "\tregister bx = %d; %s"
                 "\tregister cx = %d; %s"
                 "\tregister dx = %d; %s"
                 "\tStack_t stack[%p] -> %s;"
                 "\tcpu_canary2 = " CANARY_PRINT ";\n"
                 "CPU.stack dump \n{",
                 reason, filename, func, line, cpu, cpu->tag, cpu->cpu_canary1,
                 cpu->ax, cpu->ax == POISON ? "POISON" : "",
                 cpu->bx, cpu->bx == POISON ? "POISON" : "",
                 cpu->cx, cpu->cx == POISON ? "POISON" : "",
                 cpu->dx, cpu->dx == POISON ? "POISON" : "",
                 &cpu->stack, cpu->stack.tag, cpu->cpu_canary2);
    stack_dump(&cpu->stack, "call by CPU_dump", state, filename, func, line);
    fprintf(log, "} end of CPU Dump(%s)\n", cpu->tag);

    fclose(log);
}

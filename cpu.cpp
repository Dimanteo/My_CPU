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

void cpu_dump(CPU* cpu);

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
    soft_assert(cpu->cpu_canary1 == CANARY_VALUE, __FILE__, __PRETTY_FUNCTION__, __LINE__);
    soft_assert(cpu->cpu_canary2 == CANARY_VALUE, __FILE__, __PRETTY_FUNCTION__, __LINE__);
    stack_OK(&cpu->stack, __FILE__, __PRETTY_FUNCTION__, __LINE__);

#endif
}

void cpu_init(CPU* cpu) {
    cpu->cpu_canary1 = CANARY_VALUE;
    cpu->stack = {"stack"};
    stack_init(&cpu->stack);//default stack size = 10
    cpu->ax = POISON;
    cpu->bx = POISON;
    cpu->cx = POISON;
    cpu->dx = POISON;
    cpu->cpu_canary2 = CANARY_VALUE;
}

void cpu_destruct(CPU *cpu) {
    cpu->cpu_canary1 = 0;
    stack_destruct(&cpu->stack);
    cpu->ax = 0;
    cpu->bx = 0;
    cpu->cx = 0;
    cpu->dx = 0;
    cpu->cpu_canary2 = 0;
}
//TODO
void cpu_dump(CPU *cpu, int errcd, const char state[], const char filename[], const char func[], int line) {
    FILE* log = fopen(LOG_NAME, LOG_OPENED ? "a" : "w");
    LOG_OPENED = true;
    assert(log);

    if(cpu == nullptr) {
        fprintf(log, "Dump error_code = %d; From %s; %s (%d)\n\tCPU [NULL pointer] (%s)\n", errcd, filename, func, line, state);
    }
    fprintf(log, "CPU Dump ");
}

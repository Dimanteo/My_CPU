#include "cassert"
#include <math.h>
#include "My_Headers/protected_stack.h"
#include "My_Headers/txt_files.h"
#include "linker.h"

struct CPU {
    const char tag[MAX_NAME_LENGTH];
    canary_t cpu_canary1;
    Stack_t stack;
    double reg[4];
    canary_t cpu_canary2;
};

void cpu_init(CPU* cpu);

void cpu_destruct(CPU* cpu);

bool cpu_verify(CPU* cpu, const char filename[], const char function[], int line);

void cpu_dump(CPU* cpu, const char reason[], const char state[], const char filename[], const char func[], int line);

int main() {
    CPU cpu = {"cpu"};
    cpu_init(&cpu);
    size_t size_bin = 0;
    char* bin = read_file_to_buffer_alloc(BIN_FILE_NAME, "rb", &size_bin);
    assert(bin);
    char* pc = bin;
    FILE* cpu_out = fopen("../cpu_out.txt", "wb");
    assert(cpu_out);

    int signature = *(int*)pc;
    if (signature != SIGNATURE) {
        fprintf(stderr, "Error in CPU(%s)\nSignature mismatch SIGNATURE: %d\nActual SIGNATURE: %d.\n", cpu.tag, signature, SIGNATURE);
        FILE* log = open_stack_log();
        assert(log);
        fprintf(log, "Error in CPU(%s)\nSignature mismatch SIGNATURE: %d\nActual SIGNATURE: %d.\n", cpu.tag, signature, SIGNATURE);
        fwrite(bin, sizeof(char), size_bin, log);
        fprintf(log, "\n");
        fclose(log);
        assert(signature == SIGNATURE);
    }
    pc += sizeof(SIGNATURE);

    char version = *pc;
    if (version != VERSION) {
        fprintf(stderr, "Version mismatch. Outdated bin file.\n File VERSION: %d.\nProgram VERSION: %d.\n Compile bin and restart program.\n", version, VERSION);
        FILE* log = open_stack_log();
        assert(log);
        fprintf(log, "Error in CPU(%s)\nVersion mismatch. Outdated bin file\n File VERSION: %d.\nProgram VERSION: %d.\nCompile bin and restart program.\n", cpu.tag, version, VERSION);
        fwrite(bin, sizeof(char), size_bin, log);
        fprintf(log, "\n");
        fclose(log);
        assert(signature == SIGNATURE);
    }
    pc++;

    while (*pc != 0) {
        switch (*pc) {
#define DEF_CMD(name, code, n_arg, instructions) \
            case code:\
            {\
                int arg_v[n_arg] = {};\
                for(int i = 0; i < n_arg; i++) {\
                    arg_v[i] = *((int*)(pc + 1) + i);\
                }\
                instructions\
                pc += sizeof(int) * n_arg + sizeof(char);\
                break;\
            }
#include "commands.h"
#undef DEF_CMD
        }
        cpu_verify(&cpu, __FILE__, __PRETTY_FUNCTION__, __LINE__);
    }

    printf("oh shit, I' made it this far");
    fclose(cpu_out);
    cpu_destruct(&cpu);
    free(bin);
}

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

    FILE* log = open_stack_log();
    assert(log);

    if(cpu == nullptr) {
        fprintf(log, "Dump(%s) From %s; %s (%d)\n\tCPU [NULL pointer] (%s)\n", reason, filename, func, line, state);
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
    stack_dump(&cpu->stack, "call by CPU_dump", strcmp(state, OK_STATE) ? PARENT_OK : PARENT_ERR, filename, func, line);
    log = open_stack_log();
    assert(log);
    fprintf(log, "} end of CPU Dump(%s)\n", cpu->tag);

    fclose(log);
}

#include "cpuStruct.h"

int main() {
    CPU cpu = {"cpu"};
    cpu_init(&cpu);
    size_t size_bin = 0;
    char* bin = read_file_to_buffer_alloc(BIN_FILE_NAME, "rb", &size_bin);
    assert(bin);
    char* pc = bin;

    FILE* cpu_out = fopen(CPU_OUT_FILE, "wb");
    assert(cpu_out);
    FILE* log = fopen(CPU_LOG_NAME, "wb");
    assert(log);
    fclose(log);
    stack_reopen_log(&cpu.stack, STACK_LOG_NAME, "wb");

    int signature = *(int*)pc;
    if (signature != SIGNATURE) {
        fprintf(stderr, "Error in CPU(%s)\nSignature mismatch SIGNATURE: %d\nActual SIGNATURE: %d.\n", cpu.tag, signature, SIGNATURE);
        log = fopen(CPU_LOG_NAME, "ab");
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
        log = fopen(DEFAULT_STACK_LOG_NAME, "ab");
        assert(log);
        fprintf(log, "Error in CPU(%s)\nVersion mismatch. Outdated bin file\n File VERSION: %d.\nProgram VERSION: %d.\nCompile bin and restart program.\n", cpu.tag, version, VERSION);
        fwrite(bin, sizeof(char), size_bin, log);
        fprintf(log, "\n");
        fclose(log);
        assert(version == VERSION);
    }
    pc++;

    while (*pc != 0) {
        cpu_verify(&cpu, __FILE__, __PRETTY_FUNCTION__, __LINE__);

        switch (*pc) {
#define DEF_CMD(name, token, scanf_sample, n_arg, instructions, disasm) \
            case CMD_##name:\
            {\
                int arg_v[n_arg] = {};\
                for(int i = 0; i < n_arg; i++) {\
                    arg_v[i] = *((int*)(pc + 1) + i);\
                }\
                instructions;\
                pc += sizeof(int) * n_arg + sizeof(char);\
                break;\
            }
#include "commands.h"
#undef DEF_CMD

            default:
                fprintf(log,    "\nRuntime ERROR. Unknown command code.\npc = %d [%p]\ncode = %d", pc - bin, pc, *pc);
                fprintf(stderr, "\nRuntime ERROR. Unknown command code.\npc = %d [%p]\ncode = %d", pc - bin, pc, *pc);
                fclose(cpu_out);
                cpu_destruct(&cpu);
                free(bin);
                abort();
        }
        cpu_verify(&cpu, __FILE__, __PRETTY_FUNCTION__, __LINE__);
    }

    fclose(cpu_out);
    cpu_destruct(&cpu);
    free(bin);
}

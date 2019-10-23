#include <cstdio>
#include <cassert>
#include "linker.h"
#include "My_Headers/txt_files.h"

const char BIN_INPUT_FILENAME[] = "../bin.dak";
const char DISASM_OUTPUT_FILENAME[] = "../disAsmOutput.txt";

char* translate_code(int code);

int main() {
    size_t bin_size = 0;
    char* bin = read_file_to_buffer_alloc(BIN_INPUT_FILENAME, "rb", &bin_size);
    assert(bin);
    char* pc = bin;
    FILE* fout = fopen(DISASM_OUTPUT_FILENAME, "wb");
    assert(fout);

    int signature = *(int*)pc;
    if(SIGNATURE != signature) {
        fprintf(fout, "Signature mismatch. Choose correct bin file");
        return 0;
    }
    assert(SIGNATURE == signature);
    pc += sizeof(SIGNATURE);
    int version = *pc++;
    fprintf(fout, ";Signature: %X\n;Version: %d\n", signature, version);

    while (pc - bin != bin_size) {
        switch (*pc) {

#define DEF_CMD(name, token, scanf_sample, code, n_args, instructions, disasm) \
            case code:\
                fprintf(fout, #token disasm);\
                fprintf(fout, "\n");\
                pc += 1 + sizeof(int) * n_args;\
                break;

#include "commands.h"

#undef DEF_CMD
            default:
                fprintf(fout, "ERROR. Unknown command. Translation stopped.");
                fprintf(stderr, "ERROR. Unknown command. Translation stopped.");
                abort();
        }
    }

    fclose(fout);
    free(bin);
}

char* translate_code(int code) {
    switch (code) {
        case AX:
            return (char*)"ax";
        case BX:
            return (char*)"bx";
        case CX:
            return (char*)"cx";
        case DX:
            return (char*)"dx";
        default:
            return (char*)"UNKNOWN REGISTER";
    }
}
#include "../My_Headers/txt_files.h"
#include "../linker.h"
#include "Core.hpp"
#include "Instruction.hpp"

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <binary>\n", argv[0]);
        return EXIT_FAILURE;
    }
    size_t code_size = 0;
    char *binary = read_file_to_buffer_alloc(argv[1], "rb", &code_size);
    int code_offs = check_binary_source(binary);
    code_size -= code_offs;

    Core core;
    Tracer tracer;
    core.assignTracer(&tracer);
    core.run(binary, code_offs, code_size);

    free(binary);
}
#ifndef LINKER_H
#define LINKER_H
#include <stdio.h>

const char VERSION = 7;
const unsigned int SIGNATURE =  'kauq';
const int MAX_CMD_LEN = 10;
const size_t MAX_NAME_LEN = 20;
const int MAX_NUM_ARGS = 3;
const int RAM_SIZE = 100000;

//config file paths

//common
const char BIN_FILE_NAME[] = "bin.dak";                             // asm compilation result
//asm
const char INPUT_FILENAME[] = "asm_stdin.txt";                      // asm input file
const char LISTING_FILE[]   = "Output/compile.lst";                 // Compilation listing
//disasm
const char DISASM_OUTPUT_FILENAME[] = "Output/disasm.out";          // DisAsm output file
//cpu
const char CPU_LOG_NAME[]           = "Debug/CPU.log";              // File with CPU debug info
const char CPU_OUT_FILE[]           = "Output/CPU.out";             // CPU output file
const char STACK_LOG_NAME[]         = "Debug/Stack.log";            // CPU Stack debug info file
const char DEFAULT_STACK_LOG_NAME[] = "Stack.log";                  // Predefined stack debug info file
//translator
size_t MAX_PROG_SIZE = 1000000;
size_t CMD_BUFF_SIZE = 1000;

enum REG_CODE {
    AX = 0,
    BX = 1,
    CX = 2,
    DX = 3
};
#define DEF_CMD(name, token, scanf_samples, n_args, instructions, disasm) CMD_##name,
    enum CMD_CODE{
        #include "commands.h"
    };
#undef DEF_CMD
#endif
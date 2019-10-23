
const char VERSION = 5;
const unsigned int SIGNATURE =  'kauq';
const int MAX_CMD_LEN = 10;
const size_t MAX_NAME_LEN = 20;
const int MAX_NUM_ARGS = 1;
const char BIN_FILE_NAME[] = "../bin.dak";

enum REG_CODE {
    AX = 0, //было бы неплохо сделать 0xAAAA
    BX = 1,
    CX = 2,
    DX = 3
};
#define DEF_CMD(name, token, scanf_samples, code, n_args, instructions, disasm) CMD_##name = code,
    enum CMD_CODE{
        #include "commands.h"
    };
#undef DEF_CMD

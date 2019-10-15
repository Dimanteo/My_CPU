//
// Created by dimanteo on 11.10.2019.
//

#ifndef LINKER_H
#define LINKER_H

const char VERSION = 1;
const unsigned int SIGNATURE =  'kauq';
const int MAX_CMD_LEN = 10;
const int MAX_NUM_ARGS = 1;
enum REG_CODE {
    AX = 0xAAAA,
    BX = 0xBBBB,
    CX = 0xCCCC,
    DX = 0xDDDD
};
#define DEF_CMD(name, code, n_args) CMD_##name = code,
    enum CMD_CODE{
        #include "commands.h"
    };
#undef DEF_CMD


#endif

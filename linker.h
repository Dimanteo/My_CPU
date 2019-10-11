//
// Created by dimak on 11.10.2019.
//

#ifndef LINKER_H
#define LINKER_H

const char VERSION = 1;
const unsigned int SIGNATURE =  'mtfl';

enum CMD_CODE {END = 0, PUSH, POP, ADD, SUB, DIV, MUL, SQR, SIN, COS, XPUSH, XPOP, CMD_COUNT}; //CMD_COUNT всегда должен быть в конце enum
enum REG_CODE {AX = 0xAAAA, BX = 0xBBBB, CX = 0xCCCC, DX = 0xDDDD};
const int CMD_CODE_VALUES_BEGIN = END;
const int CMD_CODE_VALUES_END = CMD_COUNT - 1;
const size_t MAX_COMMAND_LEN = 10;
const size_t MAX_ARG_LEN = 100;

//все эти заморочки нужны для того чтобы дегко добавлять новые команды c произвольным количеством параметров
struct Command
{
    CMD_CODE code;
    size_t args_n;
    const char str[MAX_COMMAND_LEN];
};

Command cmd_list[CMD_COUNT] = {
        Command {END, 0, "end"},
        Command {PUSH, 1, "push"},
        Command {POP, 1, "pop"},
        Command {ADD, 0, "add"},
        Command {SUB, 0, "sub"},
        Command {DIV, 0, "div"},
        Command {MUL, 0, "mul"},
        Command {SQR, 0, "sqr"},
        Command {SIN, 0, "sin"},
        Command {COS, 0, "cos"},
        Command {XPUSH, 1, "push"},
        Command {XPOP, 1, "pop"}
};

#endif

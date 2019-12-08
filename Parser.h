#ifndef PROCESSOR_LABA_PARSER_H
#define PROCESSOR_LABA_PARSER_H

#include <cctype>
#include <cmath>
#include <assert.h>
#include "linker.h"

struct Parser {

    char* str = nullptr;
    int arg_v[2];
    char arg_type[2];
    int arg_num = 0;

    void skipSpaces()
    {
        while (isspace(*str))
            str++;
    }

    void getG();
    void getAdress();
    void getN();
    void getReg();
};

void Parser::getG()
{
    skipSpaces();
    if (*str == '[' )
    {
        str++;
        getAdress();
    } else {
        assert(*str == '[');
    }
    skipSpaces();
    if (*str == ']')
        assert(*str == ']');
}

void Parser::getAdress()
{
    skipSpaces();
    if (isdigit(*str))
    {
        getN();
        skipSpaces();
        if (*str == '+')
        {
            str++;
            skipSpaces();
            getReg();
        }
    } else if (isalpha(*str)) {
        getReg();
        skipSpaces();
        if (*str == '+')
        {
            str++;
            skipSpaces();
            getN();
        }
    }
}

void Parser::getReg()
{
    arg_type[arg_num] = 'r';
    switch (*str) {
        case 'a':
            str++;
            assert(*str == 'x');
            arg_v[arg_num++] = AX;
            break;
        case 'b':
            str++;
            assert(*str == 'x');
            arg_v[arg_num++] = BX;
            break;
        case 'c':
            str++;
            assert(*str == 'x');
            arg_v[arg_num++] = CX;
            break;
        case 'd':
            str++;
            assert(*str == 'x');
            arg_v[arg_num++] = DX;
            break;
    }
    str++;
}

void Parser::getN()
{
    int value = 0;
    arg_type[arg_num] = 'n';
    do {
        value = value * 10 + (*str - '0');
        str++;
    } while ('0' <= *str && *str <= '9');
    arg_v[arg_num++] = (int)value;
}
#endif //PROCESSOR_LABA_PARSER_H

#pragma once

#include "cassert"
#include <cmath>

//#define NDEBUG
//#define NO_HASH
//#define OK_DUMP
#include "linker.h"
#include "My_Headers/protected_stack.h"
#include "My_Headers/txt_files.h"

typedef int element_t;

struct CPU {
    const char tag[MAX_NAME_LENGTH];
    canary_t cpu_canary1;
    Stack_t stack;
    element_t reg[NREGS];
    int RAM[RAM_SIZE];
    bool run;
    canary_t cpu_canary2;
};

void cpu_init(CPU* cpu);

void cpu_destruct(CPU* cpu);

bool cpu_verify(CPU* cpu, const char filename[], const char function[], int line);

void cpu_dump(CPU* cpu, const char reason[], const char state[], const char filename[], const char func[], int line);

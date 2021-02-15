#pragma once
#include <string>
#include <unordered_map>
#include <iostream>

#include <llvm-10/llvm/IR/LLVMContext.h>
#include <llvm-10/llvm/IR/Module.h>
#include <llvm-10/llvm/IR/IRBuilder.h>
#include <llvm-10/llvm/IR/BasicBlock.h>
#include <llvm-10/llvm/ExecutionEngine/ExecutionEngine.h>
#include <llvm-10/llvm/ExecutionEngine/GenericValue.h>
#include <llvm-10/llvm/Support/TargetSelect.h>

#include "../linker.h"
#include "../cpuStruct.h"
#include "../My_Headers/txt_files.h"

#define STRINGIFY2(x) #x
#define STRINGIFY(x) STRINGIFY2(x)

#define DEF_CMD(CMD_name, token, scanf_sample, number_of_args, instructions, disasm_print) \
    void do_##CMD_name(CPU *cpu, const char* pc);

    #include "../commands.h"

#undef DEF_CMD

void* lazyFunctionCreator (const std::string &fname)
{
    #define DEF_CMD(CMD_name, token, scanf_sample, number_of_args, instructions, disasm_print) \
        if (fname == STRINGIFY(do_##CMD_name)) {return reinterpret_cast<void*>(do_##CMD_name);}

    #include "../commands.h"

    #undef DEF_CMD

    return nullptr;
}

std::unordered_map<CMD_CODE, std::string> cmd_code_to_func(
    {
        #define DEF_CMD(CMD_name, token, scanf_sample, number_of_args, instructions, disasm_print) \
            {CMD_##CMD_name, STRINGIFY(do_##CMD_name)},

        #include "../commands.h"

        #undef DEF_CMD
    }
);

bool is_branch(CMD_CODE code);

#pragma once
#include "Core.hpp"
#include "Instruction.hpp"
#include "types.hpp"

void gen_default(llvm::IRBuilder<> *builder, Core &core, const Insn &insn);

void gen_callback(llvm::IRBuilder<> *builder, Core &core, const Insn &insn);

void gen_end(llvm::IRBuilder<> *builder, Core &core, const Insn &insn);

void gen_jump(llvm::IRBuilder<> *builder, Core &core, const Insn &insn);

void gen_jumpa(llvm::IRBuilder<> *builder, Core &core, const Insn &insn);
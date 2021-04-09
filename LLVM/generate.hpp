#pragma once
#include "Core.hpp"
#include "Instruction.hpp"
#include "types.hpp"

extern std::unordered_map<std::string, Insn::execFunc_t> functionCreatorMap;

void gen_default(llvm::IRBuilder<> *builder, const Core &core,
                 const Insn &insn);

void gen_callback(llvm::IRBuilder<> *builder, const Core &core,
                  const Insn &insn);

void gen_end(llvm::IRBuilder<> *builder, const Core &core, const Insn &insn);
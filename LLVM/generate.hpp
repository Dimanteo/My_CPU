#pragma once
#include "Core.hpp"
#include "Instruction.hpp"
#include "types.hpp"

void gen_default(llvm::IRBuilder<> *builder, const Core &core,
                 const Insn &insn) {
    llvm::FunctionType *calleeT = llvm::FunctionType::get(
        builder->getVoidTy(),
        llvm::ArrayRef<llvm::Type *>(
            {builder->getInt8PtrTy(), builder->getInt8PtrTy()}),
        false);
    llvm::Value *core_ptr = llvm::ConstantInt::get(
        builder->getInt64Ty(), reinterpret_cast<uint64_t>(&core));
    llvm::Value *insn_ptr = llvm::ConstantInt::get(
        builder->getInt64Ty(), reinterpret_cast<uint64_t>(&insn));
    builder->CreateCall(
        core.getModule()->getOrInsertFunction(insn.getName(), calleeT),
        llvm::ArrayRef<llvm::Value *>({core_ptr, insn_ptr}));
}

void gen_end(llvm::IRBuilder<> *builder, const Core &core, const Insn &insn) {
    builder->CreateRetVoid();
}

#include "generate.hpp"
#include "Core.hpp"
#include "Instruction.hpp"
#include "types.hpp"

void gen_default(llvm::IRBuilder<> *builder, Core &core, const Insn &insn) {
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

void gen_callback(llvm::IRBuilder<> *builder, Core &core, const Insn &insn) {
    llvm::FunctionType *calleeT = llvm::FunctionType::get(
        builder->getVoidTy(),
        llvm::ArrayRef<llvm::Type *>(
            {builder->getInt8PtrTy(), builder->getInt8PtrTy()}),
        false);
    llvm::Value *core_ptr = llvm::ConstantInt::get(
        builder->getInt64Ty(), reinterpret_cast<uint64_t>(&core));
    llvm::Value *insn_ptr = llvm::ConstantInt::get(
        builder->getInt64Ty(), reinterpret_cast<uint64_t>(&insn));
    builder->CreateCall(core.getModule()->getOrInsertFunction("trace", calleeT),
                        llvm::ArrayRef<llvm::Value *>({core_ptr, insn_ptr}));
}

llvm::Value *ir_pop(llvm::IRBuilder<> *builder, Core &core) {
    llvm::FunctionType *calleeT = llvm::FunctionType::get(
        builder->getInt32Ty(),
        llvm::ArrayRef<llvm::Type *>({builder->getInt8PtrTy()}), false);
    llvm::Value *core_ptr = llvm::ConstantInt::get(
        builder->getInt64Ty(), reinterpret_cast<uint64_t>(&core));
    llvm::Value *retval = builder->CreateCall(
        core.getModule()->getOrInsertFunction("pop", calleeT),
        llvm::ArrayRef<llvm::Value *>({core_ptr}));
    return retval;
}

void gen_end(llvm::IRBuilder<> *builder, Core &core, const Insn &insn) {
    builder->CreateRetVoid();
}

void gen_jump(llvm::IRBuilder<> *builder, Core &core, const Insn &insn) {
    llvm::BasicBlock *bb = core.getBasicBlock(insn.getArg(0));
    builder->CreateBr(bb);
}

void gen_jumpa(llvm::IRBuilder<> *builder, Core &core, const Insn &insn) {
    llvm::Value *lhs = ir_pop(builder, core);
    llvm::Value *rhs = ir_pop(builder, core);
    llvm::Value *cond = builder->CreateICmpSGT(lhs, rhs);
    llvm::BasicBlock *trueDest = core.getBasicBlock(insn.getArg(0));
    llvm::BasicBlock *falseDest =
        core.getBasicBlock(core.getPC() + insn.getSz());
    builder->CreateCondBr(cond, trueDest, falseDest);
}

void gen_jumpae(llvm::IRBuilder<> *builder, Core &core, const Insn &insn) {
    llvm::Value *lhs = ir_pop(builder, core);
    llvm::Value *rhs = ir_pop(builder, core);
    llvm::Value *cond = builder->CreateICmpSGE(lhs, rhs);
    llvm::BasicBlock *trueDest = core.getBasicBlock(insn.getArg(0));
    llvm::BasicBlock *falseDest =
        core.getBasicBlock(core.getPC() + insn.getSz());
    builder->CreateCondBr(cond, trueDest, falseDest);
}

void gen_jumpb(llvm::IRBuilder<> *builder, Core &core, const Insn &insn) {
    llvm::Value *lhs = ir_pop(builder, core);
    llvm::Value *rhs = ir_pop(builder, core);
    llvm::Value *cond = builder->CreateICmpSLT(lhs, rhs);
    llvm::BasicBlock *trueDest = core.getBasicBlock(insn.getArg(0));
    llvm::BasicBlock *falseDest =
        core.getBasicBlock(core.getPC() + insn.getSz());
    builder->CreateCondBr(cond, trueDest, falseDest);
}

void gen_jumpbe(llvm::IRBuilder<> *builder, Core &core, const Insn &insn) {
    llvm::Value *lhs = ir_pop(builder, core);
    llvm::Value *rhs = ir_pop(builder, core);
    llvm::Value *cond = builder->CreateICmpSLE(lhs, rhs);
    llvm::BasicBlock *trueDest = core.getBasicBlock(insn.getArg(0));
    llvm::BasicBlock *falseDest =
        core.getBasicBlock(core.getPC() + insn.getSz());
    builder->CreateCondBr(cond, trueDest, falseDest);
}

void gen_jumpeq(llvm::IRBuilder<> *builder, Core &core, const Insn &insn) {
    llvm::Value *lhs = ir_pop(builder, core);
    llvm::Value *rhs = ir_pop(builder, core);
    llvm::Value *cond = builder->CreateICmpEQ(lhs, rhs);
    llvm::BasicBlock *trueDest = core.getBasicBlock(insn.getArg(0));
    llvm::BasicBlock *falseDest =
        core.getBasicBlock(core.getPC() + insn.getSz());
    builder->CreateCondBr(cond, trueDest, falseDest);
}

void gen_jumpne(llvm::IRBuilder<> *builder, Core &core, const Insn &insn) {
    llvm::Value *lhs = ir_pop(builder, core);
    llvm::Value *rhs = ir_pop(builder, core);
    llvm::Value *cond = builder->CreateICmpNE(lhs, rhs);
    llvm::BasicBlock *trueDest = core.getBasicBlock(insn.getArg(0));
    llvm::BasicBlock *falseDest =
        core.getBasicBlock(core.getPC() + insn.getSz());
    builder->CreateCondBr(cond, trueDest, falseDest);
}
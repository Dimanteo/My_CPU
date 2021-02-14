#include "LLVM_IR.hpp"

int main(int argc, char* argv[])
{
    if (argc < 2)
    {
        fprintf(stderr, "Usage: %s <binary> [<file>]\n", argv[0]);
        return EXIT_FAILURE;
    }

    char* bin_file = argv[1];
    std::string ir_file("llvmir.ll");
    if (argc > 2)
        ir_file = argv[2];

    llvm::LLVMContext context;
    llvm::Module module("top", context);
    llvm::IRBuilder<> builder(context);

    llvm::FunctionType *funcType = 
        llvm::FunctionType::get(builder.getVoidTy(), false);
    llvm::Function *mainFunc = 
        llvm::Function::Create(funcType, llvm::Function::ExternalLinkage, "main", module);
    llvm::BasicBlock* entry = llvm::BasicBlock::Create(context, "start", mainFunc);

    builder.SetInsertPoint(entry);

    size_t code_size = 0;
    char *binary = read_file_to_buffer_alloc(argv[1], "rb", &code_size);
    int code_offs = check_binary_source(binary);
    char *pc = binary + code_offs;

    CPU cpu = {"IR CPU"};
    cpu_init(&cpu);

    std::unordered_map<char*, llvm::BasicBlock*> addr_to_block;
    for (; pc - binary < code_size; pc++)
    {
        CMD_CODE code = static_cast<CMD_CODE>(*pc);

        if (code == CMD_JUMP) 
        {
            int dest = *(int*)(pc + 1);
            char* next_pc = pc + 1 + sizeof(int);
            size_t next_insn =  next_pc  - binary;
            addr_to_block.insert({binary + dest, llvm::BasicBlock::Create(context, std::to_string(dest), mainFunc)});
            addr_to_block.insert({next_pc, llvm::BasicBlock::Create(context, std::to_string(next_insn), mainFunc)});
        }

        switch(code)
        {
            #define DEF_CMD(CMD_name, token, scanf_sample, number_of_args, instructions, disasm_print) \
                case CMD_##CMD_name: \
                    pc += number_of_args * sizeof(int); \
                    break;

            #include "../commands.h"

            #undef DEF_CMD
        }
    }

    pc = binary + code_offs;

    llvm::FunctionType *calleType = llvm::FunctionType::get(builder.getVoidTy(),
                                        llvm::ArrayRef<llvm::Type*>({builder.getInt8PtrTy(), builder.getInt8PtrTy()}), false);
    llvm::Value *cpu_ptr = llvm::ConstantInt::get(builder.getInt64Ty(), reinterpret_cast<uint64_t>(&cpu));

    for (; pc - binary < code_size; pc++)
    {
        CMD_CODE code = static_cast<CMD_CODE>(*pc);

        if (code == CMD_END)
        {
            builder.CreateRetVoid();
            continue;
        }

        auto block_it = addr_to_block.find(pc);
        if (block_it != addr_to_block.end())
        {
            builder.CreateBr(block_it->second);
            builder.SetInsertPoint(block_it->second);
        }

        switch (code)
        {
            case CMD_JUMP:
            {
                char* dest_pc = *(int*)(pc + 1) + binary;
                builder.CreateBr(addr_to_block[dest_pc]);
                break;
            }
            default:
            {
                llvm::Value *inst_ptr = llvm::ConstantInt::get(builder.getInt64Ty(), reinterpret_cast<uint64_t>(pc));
                builder.CreateCall(module.getOrInsertFunction(cmd_code_to_func[code], calleType), llvm::ArrayRef<llvm::Value*>({cpu_ptr, inst_ptr}));
                break;
            }
        }
    
        switch(code)
        {
            #define DEF_CMD(CMD_name, token, scanf_sample, number_of_args, instructions, disasm_print) \
                case CMD_##CMD_name: \
                    pc += number_of_args * sizeof(int); \
                    break;

            #include "../commands.h"

            #undef DEF_CMD
        }
    }

    builder.CreateRetVoid();

    std::cout << "## [LLVM IR] DUMP ##\n";

    std::string ir_str;
    llvm::raw_string_ostream ir_os(ir_str);
    module.print(ir_os, nullptr);
    ir_os.flush();
    std::cout << ir_str;

    std::cout << "## [LLVM EE] RUN ##\n";

    llvm::InitializeNativeTarget();
    llvm::InitializeNativeTargetAsmPrinter();

    llvm::ExecutionEngine *ee = llvm::EngineBuilder(std::unique_ptr<llvm::Module>(&module)).create();
    ee->InstallLazyFunctionCreator(lazyFunctionCreator);
    ee->finalizeObject();

    std::vector<llvm::GenericValue> noargs;
    cpu.run = true;
    ee->runFunction(mainFunc, noargs);

    std::cout << "## [LLVM EE] END ##\n";

    cpu_destruct(&cpu);
    free(binary);
}

void do_END(CPU *cpu, const char* pc)
{
    cpu->run = false;
}

void do_PUSH(CPU *cpu, const char* pc) 
{
    stack_push(&cpu->stack, *(int*)(pc + 1));
}

void do_POP(CPU *cpu, const char* pc) 
{
    int reg_i = *(int*)(pc + 1);
    cpu->reg[reg_i] = stack_pop(&cpu->stack);
}

void do_ADD(CPU *cpu, const char* pc)
{
    int arg1 = stack_pop(&cpu->stack);
    int arg2 = stack_pop(&cpu->stack);
    stack_push(&cpu->stack, arg1 + arg2);
}

void do_SUB(CPU *cpu, const char* pc)
{
    int rhs = stack_pop(&cpu->stack);
    int lhs = stack_pop(&cpu->stack);
    stack_push(&cpu->stack, lhs - rhs);
}

void do_MUL(CPU *cpu, const char* pc)
{
    int arg1 = stack_pop(&cpu->stack);
    int arg2 = stack_pop(&cpu->stack);
    stack_push(&cpu->stack, arg1 * arg2 / PRECISION);
}

void do_DIV(CPU *cpu, const char* pc)
{
    float rhs = static_cast<float>(stack_pop(&cpu->stack));
    float lhs = static_cast<float>(stack_pop(&cpu->stack));
    stack_push(&cpu->stack, static_cast<int>(lhs / rhs * PRECISION));
}

void do_SQR(CPU *cpu, const char* pc)
{
    double val = static_cast<double>(stack_pop(&cpu->stack));
    int res = static_cast<int>(sqrt(val / PRECISION) * PRECISION);
    stack_push(&cpu->stack, res);
}
void do_SIN(CPU *cpu, const char* pc)
{
    double val = static_cast<double>(stack_pop(&cpu->stack));
    int res = static_cast<int>(sin(val / PRECISION) * PRECISION);
    stack_push(&cpu->stack, res);
}
void do_COS(CPU *cpu, const char* pc)
{
    double val = static_cast<double>(stack_pop(&cpu->stack));
    int res = static_cast<int>(cos(val / PRECISION) * PRECISION);
    stack_push(&cpu->stack, res);
}
void do_OUT(CPU *cpu, const char* pc)
{
float val = static_cast<float>(stack_pop(&cpu->stack)) / PRECISION;
    printf("%.3g\n", val);
}
void do_IN(CPU *cpu, const char* pc)
{
    float val = 0;
    scanf("%f", &val);
    stack_push(&cpu->stack, static_cast<int>(val * PRECISION));
}

void do_PUSHX(CPU *cpu, const char* pc) 
{
    int reg_i = *(int*)(pc + 1);
    stack_push(&cpu->stack, cpu->reg[reg_i]);
}

void do_JUMP(CPU *cpu, const char* pc) {}
void do_JUMPA(CPU *cpu, const char* pc) {}
void do_JUMPAE(CPU *cpu, const char* pc) {}
void do_JUMPB(CPU *cpu, const char* pc) {}
void do_JUMPBE(CPU *cpu, const char* pc) {}
void do_JUMPE(CPU *cpu, const char* pc) {}
void do_JUMPNE(CPU *cpu, const char* pc) {}
void do_CALL(CPU *cpu, const char* pc) {}
void do_RET(CPU *cpu, const char* pc) {}
void do_POPRAM_NX(CPU *cpu, const char* pc) {}
void do_POPRAM_XN(CPU *cpu, const char* pc) {}
void do_POPRAM(CPU *cpu, const char* pc) {}
void do_PUSHRAM(CPU *cpu, const char* pc) {}
void do_POPRAM_X(CPU *cpu, const char* pc) {}
void do_PUSHRAM_X(CPU *cpu, const char* pc) {}
void do_PUSHRAM_NX(CPU *cpu, const char* pc) {}
void do_PUSHRAM_XN(CPU *cpu, const char* pc) {}

void do_POWER(CPU *cpu, const char* pc)
{
    double power = static_cast<double>(stack_pop(&cpu->stack)) / PRECISION;
    double base  = static_cast<double>(stack_pop(&cpu->stack)) / PRECISION;
    int res = static_cast<int>(pow(base, power) * PRECISION);
    stack_push(&cpu->stack, res);
}

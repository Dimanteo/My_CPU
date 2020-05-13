#include "translator_x86_64.h"


int main(int argc, char* argv[])
{
    REGIMES regime = BINARY;
    char* out_filename = nullptr;
    if (argc > 1)
    {
        if (strcmp(argv[1], "-s") == 0)
        {
            regime = TEXT;
        } else {
            printf("Incorrect translator flag: %s\n", argv[1]);
            return 1;
        }
        if (argc > 2)
        {
            out_filename = argv[2];
        } else {
            out_filename = "b.out";
        }
    }

    char* result = translate(BIN_FILE_NAME, regime);
    assert(result);
    
    if (regime == TEXT)
    {
        plain_print(out_filename, result);
    } else {
        make_elf(out_filename, result);
    }
    free(result);
    return 0;
}


Command_x86_64::Command_x86_64(REGIMES regime) : _regime(regime)
{
    _body = (char*)calloc(CMD_BUFF_SIZE, sizeof(_body[0]));
    _size = 0;
}


Command_x86_64::~Command_x86_64() 
{
    free(_body);
    _body = nullptr;
    _size = 0;
}


char* Command_x86_64::get_body()
{
    return _body;
}


void Command_x86_64::make_label(int pc, char* label)
{
    assert(label);
    sprintf(label, ".LBL_%d", pc);
}


void Command_x86_64::get_args(char* operand, int* buff, int nargs)
{
    assert(operand);
    assert(buff);

    operand++;
    for (int i = 0; i < nargs; i++)
    {
        buff[i] = ((int*)operand)[i];
    }
}


void Command_x86_64::match_reg(int code, char* reg)
{
    assert(reg);
    switch (code)
    {
        case AX:
            strcpy(reg, "r8");
            break;
        case BX:
            strcpy(reg, "r9");
            break;
        case CX:
            strcpy(reg, "r10");
            break;
        case DX:
            strcpy(reg, "r11");
            break;
        default:
#ifndef NDEBUG
            fprintf(stderr, "Unknown register");
#endif
            break;
    }
}

#define LBL _body, "%s:\n"
#define GETARGS(n)  nargs = n ;\
                    int args[nargs] = {};\
                    get_args(src + pc, args, nargs);

int Command_x86_64::translate_cmd(char* src, int pc)
{
    assert(src);

    char label[CMD_BUFF_SIZE] = {0};
    make_label(pc, label);
    char code = src[pc];
    int nargs = 0;
    switch (code)
    {
        case CMD_END:
        {
            nargs = 0;
            sprintf(LBL
                "; END\n"
                "\tmov rax, 0x3C\n"
                "\tmov rdi, 0\n"
                "\tsyscall\n",
                label);
            break;
        }
        case CMD_PUSH:
        {
            GETARGS(1);
            sprintf(LBL
                "; PUSH\n"
                "\tpush %d\n",
                label,
                args[0]);
            break;
        }
        case CMD_POP:
        {
            GETARGS(1)
            char reg[4] = {0};
            match_reg(args[0], reg);
            sprintf(LBL
                "; POP\n"
                "\tpop %s\n",
                label,
                reg);
            break;
        }
        case CMD_ADD:
        {
            nargs = 0;
            sprintf(LBL
                "; ADD\n"
                "\tpop rsi\n"
                "\tpop rdi\n"
                "\tadd rdi, rsi\n"
                "\tpush rdi\n",
                label);
                break;
        }
        case CMD_SUB:
        {
            nargs = 0;
            sprintf(LBL
                "; SUB\n"
                "\tpop rsi\n"
                "\tpop rdi\n"
                "\tsub rsi, rdi\n"
                "\tpush rsi\n",
                label);
                break;
        }
        case CMD_MUL:
        {
            nargs = 0;
            sprintf(LBL
                "; MUL\n"
                "\tpush 1000\n"
                "\tfild qword [rsp+16]\n"
                "\tfild qword [rsp+8]\n"
                "\tfild qword [rsp]\n"
                "\tadd rsp, 16\n"
                "\tfdiv\n"
                "\tfmul\n"
                "\tfistp qword [rsp]\n",
                label);
                break;
        }
        case CMD_DIV:
        {
            nargs = 0;
            sprintf(LBL
                "; DIV\n"
                "\tpush 1000\n"
                "\tfild qword [rsp]\n"
                "\tfild qword [rsp+8]\n"
                "\tfild qword [rsp+16]\n"
                "\tadd rsp, 16\n"
                "\tfdiv\n"
                "\tfmul\n"
                "\tfistp qword [rsp]\n",
                label);
                break;
        }
        case CMD_SQR:
        case CMD_SIN:
        case CMD_COS:
        {
            char instr[6] = {0};
            if (code == CMD_SQR)
            {
                strcpy(instr, "fsqrt");
            } else if (code == CMD_SIN) {
                strcpy(instr, "fsin");
            } else {
                strcpy(instr, "fcos");
            }
            nargs = 0;
            sprintf(LBL 
                "; SQR | SIN | COS\n"
                "\tpush 1000\n"
                "\tfild qword [rsp]\n"
                "\tfild qword [rsp+8]\n"
                "\tfild qword [rsp]\n"
                "\tadd rsp, 8\n"
                "\tfdiv\n"
                "\t%s\n"
                "\tfmul\n"
                "\tfistp qword [rsp]\n",
                label,
                instr);
            break;
        }
        case CMD_IN:
        {
            nargs = 0;
            sprintf(LBL
                "; IN\n"
                "\tcall stdIN\n"
                "\tpush rax\n",
                label);
            break;
        }
        case CMD_OUT:
        {
            nargs = 0;
            sprintf(LBL 
                "; OUT\n"
                "\tcall stdOUT\n",
                label);
            break;
        }
        case CMD_PUSHX:
        {
            GETARGS(1)
            char reg[4] = {0};
            match_reg(args[0], reg);
            sprintf(LBL
                "; PUSH_X\n"
                "\tpush %s\n",
                label,
                reg);
            break;
        }
        case CMD_JUMP:
        {
            GETARGS(1)
            char jmp_dest[CMD_BUFF_SIZE] = {0};
            make_label(args[0], jmp_dest);
            sprintf(LBL
                "; JMP\n"
                "\tjmp %s\n",
                label,
                jmp_dest);
            break;
        }
        case CMD_JUMPA:
        case CMD_JUMPAE:
        case CMD_JUMPB:
        case CMD_JUMPBE:
        case CMD_JUMPE:
        case CMD_JUMPNE:
        {
            char instr[4] = {0};
            switch(code)
            {
                case CMD_JUMPA:
                    strcpy(instr, "ja");
                    break;
                case CMD_JUMPAE:
                    strcpy(instr, "jae");
                    break;
                case CMD_JUMPB:
                    strcpy(instr, "jb");
                    break;
                case CMD_JUMPBE:
                    strcpy(instr, "jbe");
                    break;
                case CMD_JUMPE:
                    strcpy(instr, "je");
                    break;
                case CMD_JUMPNE:
                    strcpy(instr, "jne");
                    break;
            }
            GETARGS(1)
            char jmp_dest[CMD_BUFF_SIZE] = {0};
            make_label(args[0], jmp_dest);
            sprintf(LBL
                "; Jxx\n"
                "\tpop rdi\n"
                "\tpop rsi\n"
                "\tcmp rdi, rsi\n"
                "\t%s %s\n",
                label,
                instr, jmp_dest);
            break;
        }
        case CMD_CALL:
        {
            GETARGS(1)
            char call_dest[CMD_BUFF_SIZE] = {0};
            make_label(args[0], call_dest);
            sprintf(LBL
                "; CALL\n"
                "\tcall %s\n",
                label,
                call_dest);
            break;
        }
        case CMD_RET:
        {
            nargs = 0;
            sprintf(LBL
                "; RET\n"
                "\tret\n",
                label);
            break;
        }
        case CMD_PUSHRAM:
        {
            GETARGS(1)
            sprintf(LBL
                "; PUSHRAM\n"
                "\tpop rax\n"
                "\tmov qword [rbp + %d * %d], rax\n",
                label,
                sizeof(int),
                args[0]);
            break;
        }
        case CMD_POPRAM:
        {
            GETARGS(1)
            sprintf(LBL
                "; POPRAM\n"
                "\tmov rax, [rbp + %d * %d]\n"
                "\tpush rax\n",
                label,
                sizeof(int),
                args[0]);
            break;
        }
        case CMD_PUSHRAM_X:
        {
            GETARGS(1)
            char reg[4] = {0};
            match_reg(args[0], reg);
            sprintf(LBL
                "; PUSHRAM_X\n"
                "\tmov rax, %s\n"
                "\tmov rbx, 1000\n"
                "\tdiv rbx\n"
                "\tpop rbx\n"
                "\tmov qword [rbp + %d * rax], rbx\n",
                label,
                reg,
                sizeof(int));
            break;
        }
        case CMD_POPRAM_X:
        {
            GETARGS(1)
            char reg[4] = {0};
            match_reg(args[0], reg);
            sprintf(LBL
                "; POPRAM_X\n"
                "\tmov rax, %s\n"
                "\tmov rbx, 1000\n"
                "\tdiv rbx\n"
                "\tmov rax, qword [rbp + %d * rax]\n"
                "\tpush rax\n",
                label,
                reg,
                sizeof(int));
            break;
        }
        case CMD_PUSHRAM_NX:
        {
            GETARGS(2)
            char reg[4] = {0};
            match_reg(args[1], reg);
            sprintf(LBL
                "; PUSHRAM_NX\n"
                "\tmov rax, %s\n"
                "\tmov rbx, 1000\n"
                "\tdiv rbx\n"
                "\tpop rbx\n"
                "\tmov qword [rbp + %d * rax + %d * %d], rbx\n",
                label,
                reg,
                sizeof(int),
                sizeof(int),
                args[0]);
            break;
        }
        case CMD_PUSHRAM_XN:
        {
            GETARGS(2)
            char reg[4] = {0};
            match_reg(args[0], reg);
            sprintf(LBL
                "; PUSHRAM_XN\n"
                "\tmov rax, %s\n"
                "\tmov rbx, 1000\n"
                "\tdiv rbx\n"
                "\tpop rbx\n"
                "\tmov qword [rbp + %d * rax + %d * %d], rbx\n",
                label,
                reg,
                sizeof(int),
                sizeof(int),
                args[1]);
            break;
        }
        case CMD_POPRAM_NX:
        {
            GETARGS(2)
            char reg[4] = {0};
            match_reg(args[1], reg);
            sprintf(LBL
                "; POPRAM NX\n"
                "\tmov rax, %s\n"
                "\tmov rbx, 1000\n"
                "\tdiv rbx\n"
                "\tmov rax, [rbp + %d * rax + %d * %d]\n"
                "\tpush rax\n",
                label,
                reg,
                sizeof(int),
                sizeof(int),
                args[0]);
            break;
        }
        case CMD_POPRAM_XN:
        {
            GETARGS(2)
            char reg[4] = {0};
            match_reg(args[0], reg);
            sprintf(LBL
                "; POPRAM_XN\n"
                "\tmov rax, %s\n"
                "\tmov rbx, 1000\n"
                "\tdiv rbx\n"
                "\tmov rax, [rbp+ %d * rax + %d * %d]\n"
                "\tpush rax\n",
                label,
                sizeof(int),
                sizeof(int),
                reg,
                args[1]);
            break;
        }
    }
    pc += 1 + nargs * sizeof(int); 
    return pc;
}


int check_source(char* buff)
{
    assert(buff);
    int pc = 0;
    int signature = *(int*)buff;
    if (signature != SIGNATURE) {
        fprintf(stderr, "ERROR in translator. Signature mismatch.\nExpected: %d\nGot: %d\n", SIGNATURE, signature);
        assert(signature == SIGNATURE);
    }
    pc += sizeof(SIGNATURE);

    char version = buff[pc];
    if (version != VERSION) {
        fprintf(stderr, "ERROR in translator. Version mismatch.\n File VERSION: %d.\nProgram VERSION: %d.\n Recompile bin and restart program.\n", version, VERSION);
        assert(version == VERSION);
    }
    return ++pc;
}


void make_header(char* dst, REGIMES regime)
{
    if (regime == TEXT)
    {
        time_t now = time(nullptr);
        sprintf(dst, 
            "; Translation %s\n\n"
            "global _start\n\n"
            "section .text\n\n"
            "_start:\n\n"
            "; Init header\n\n"
            "\tsub rsp, %d\t; RAM init\n"
            "\tmov rbp, rsp\t; Save RAM adress\n"
            "\tfinit\n\n"
            "; Translated text start\n\n",
            ctime(&now), RAM_SIZE * sizeof(int));
    }
}


char* translate(const char* bin_file, REGIMES regime)
{
    size_t src_size = 0;
    char* src = read_file_to_buffer_alloc(bin_file, "rb", &src_size);
    int pc = check_source(src);
    char* dst = (char*)calloc(MAX_PROG_SIZE, sizeof(dst[0]));
    make_header(dst, regime);

    Command_x86_64 command(regime);
    while (pc < src_size)
    {
        pc = command.translate_cmd(src, pc);
        strcat(dst, command.get_body());
    }
    free(src);
    return dst;
}


void plain_print(const char* filename, const char* text)
{
    assert(filename);
    assert(text);
    FILE* file = fopen(filename, "wb");
    assert(file);
    fwrite(text, sizeof(text[0]), strlen(text), file);
    fclose(file);
}


void make_elf(const char* filename, const char* body){}

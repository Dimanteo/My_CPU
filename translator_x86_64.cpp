#include "translator_x86_64.h"


int main(int argc, char* argv[])
{
    REGIMES regime = BINARY;
    char* out_filename = nullptr;
    if (argc > 1)
    {
        if (strcmp(argv[1], "-s"))
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
    
    if (regime == TEXT)
    {
        plain_print(out_filename, result);
    } else {
        make_elf(out_filename, result);
    }
    return 0;
}


Command_x86_64::Command_x86_64(REGIMES regime) : _regime(regime)
{
    _body = (char*)calloc(CMD_BUFF_SIZE, sizeof(_body[0]));
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
    sprintf(label, ".LBL_%d", pc);
}


void Command_x86_64::get_args(char* operand, int* buff, int nargs)
{
    operand += 1;
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

int Command_x86_64::translate_single(char* src, int pc)
{
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
                "mov rax, 0x3C\n"
                "mov rdi, 0\n"
                "syscall\n",
                label);
            break;
        }
        case CMD_PUSH:
        {
            GETARGS(1);
            sprintf(LBL
                "push %d\n",
                label,
                (float)args[0] / 1000);
            break;
        }
        case CMD_POP:
        {
            GETARGS(1)
            char reg[4] = {0};
            match_reg(args[0], reg);
            sprintf(LBL
                "pop %s\n",
                label,
                reg);
            break;
        }
        case CMD_ADD:
        {
            nargs = 0;
            sprintf(LBL
                "pop rsi\n"
                "pop rdi\n"
                "add rdi, rsi\n"
                "push rdi\n",
                label);
                break;
        }
        case CMD_SUB:
        {
            nargs = 0;
            sprintf(LBL
                "pop rsi\n"
                "pop rdi\n"
                "sub rsi, rdi\n"
                "push rsi\n",
                label);
                break;
        }
        case CMD_MUL:
        {
            nargs = 0;
            sprintf(LBL
                "push 1000\n"
                "fild qword [rsp+16]\n"
                "fild qword [rsp+8]\n"
                "fild qword [rsp]\n"
                "add rsp, 16\n"
                "fdiv\n"
                "fmul\n"
                "fistp qword [rsp]\n",
                label);
                break;
        }
        case CMD_DIV:
        {
            nargs = 0;
            sprintf(LBL
                "push 1000\n"
                "fild qword [rsp]\n"
                "fild qword [rsp+8]\n"
                "fild qword [rsp+16]\n"
                "add rsp, 16\n"
                "fdiv\n"
                "fmul\n"
                "fistp qword [rsp]\n",
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
                "push 1000\n"
                "fild qword [rsp]\n"
                "fild qword [rsp+8]\n"
                "fild qword [rsp]\n"
                "add rsp, 8\n"
                "fdiv\n"
                "%s\n"
                "fmul\n"
                "fistp qword [rsp]\n",
                label,
                instr);
            break;
        }
        case CMD_IN:
        {
            nargs = 0;
            sprintf(LBL
                "call stdIN\n"
                "add rsp, 8\n",
                label);
            break;
        }
        case CMD_OUT:
        {
            nargs = 0;
            sprintf(LBL 
                "call stdOUT\n"
                "add rsp, 8\n",
                label);
            break;
        }
        case CMD_PUSHX:
        {
            GETARGS(1)
            char reg[4] = {0};
            match_reg(args[0], reg);
            sprintf(LBL
                "push %s\n",
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
                "jmp %s\n",
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
                "pop rdi\n"
                "pop rsi\n"
                "cmp rdi, rsi\n"
                "%s %s\n",
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
                "call %s\n",
                label,
                call_dest);
            break;
        }
        case CMD_RET:
        {
            nargs = 0;
            sprintf(LBL
                "ret\n",
                label);
            break;
        }
        case CMD_PUSHRAM:
        {
            GETARGS(1)
            sprintf(LBL
                "pop rax\n"
                "mov qword [r12+%d], rax\n",
                label,
                args[0]);
            break;
        }
        case CMD_POPRAM:
        {
            GETARGS(1)
            sprintf(LBL
                "mov rax, [r12+%d]\n"
                "push rax\n",
                label,
                args[0]);
            break;
        }
        case CMD_PUSHRAM_X:
        {
            GETARGS(1)
            char reg[4] = {0};
            match_reg(args[0], reg);
            sprintf(LBL
                "mov rax, %s\n"
                "mov rbx, 1000\n"
                "div rbx\n"
                "pop rbx\n"
                "mov qword [r12 + rax], rbx\n",
                label,
                reg);
            break;
        }
        case CMD_POPRAM_X:
        {
            GETARGS(1)
            char reg[4] = {0};
            match_reg(args[0], reg);
            sprintf(LBL
                "mov rax, %s\n"
                "mov rbx, 1000\n"
                "div rbx\n"
                "mov rax, qword [r12 + rax]\n"
                "push rax\n",
                label,
                reg);
            break;
        }
        case CMD_PUSHRAM_NX:
        {
            GETARGS(2)
            char reg[4] = {0};
            match_reg(args[1], reg);
            sprintf(LBL
                "mov rax, %s\n"
                "mov rbx, 1000\n"
                "div rbx\n"
                "pop rbx\n"
                "mov qword [r12+rax+%d], rbx\n",
                label,
                reg,
                args[0]);
            break;
        }
        case CMD_PUSHRAM_XN:
        {
            GETARGS(2)
            char reg[4] = {0};
            match_reg(args[0], reg);
            sprintf(LBL
                "mov rax, %s\n"
                "mov rbx, 1000\n"
                "div rbx\n"
                "pop rbx\n"
                "mov qword [r12+rax+%d], rbx\n",
                label,
                reg,
                args[1]);
            break;
        }
        case CMD_POPRAM_NX:
        {
            GETARGS(2)
            char reg[4] = {0};
            match_reg(args[1], reg);
            sprintf(LBL
                "mov rax, %s\n"
                "mov rbx, 1000\n"
                "div rbx\n"
                "mov rax, [r12+rax+%d]\n"
                "push rax\n",
                label,
                reg,
                args[0]);
            break;
        }
        case CMD_POPRAM_XN:
        {
            GETARGS(2)
            char reg[4] = {0};
            match_reg(args[0], reg);
            sprintf(LBL
                "mov rax, %s\n"
                "mov rbx, 1000\n"
                "div rbx\n"
                "mov rax, [r12+rax+%d]\n"
                "push rax\n",
                label,
                reg,
                args[1]);
            break;
        }
    }
    free(label);
    pc += 1 + nargs * sizeof(int); 
}


char* translate(const char* bin_file, REGIMES regime)
{
    size_t src_size = 0;
    char* src = read_file_to_buffer_alloc(bin_file, "rb", &src_size);
    char* dst = (char*)calloc(MAX_PROG_SIZE, sizeof(dst[0]));
    Command_x86_64 command(regime);
    for (int pc = 0; pc < src_size;)
    {
        pc = command.translate_single(src, pc);
        strcat(dst, command.get_body());
    }
    free(src);
}

void plain_print(const char* filename, const char* text){}

void make_elf(const char* filename, const char* body){}

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
    sprintf(label, "LBL_%d", pc);
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
            reg = "r8";
            break;
        case BX:
            reg = "r9";
            break;
        case CX:
            reg = "r10";
            break;
        case DX:
            reg = "r11";
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
                (float)args[0] / 1000);
            break;
        }
        case CMD_POP:
        {
            GETARGS(1)
            char reg[4] = {0};
            match_reg(args[0], reg);
            sprintf(LBL,
                "pop %s\n",
                reg);
            break;
        }
        case CMD_ADD:
        {
            nargs = 0;
            sprintf(LBL,
                "pop rsi\n"
                "pop rdi\n"
                "add rdi, rsi\n"
                "push rdi");
                break;
        }
        case CMD_SUB:
        {
            nargs = 0;
            sprintf(LBL,
                "pop rsi\n"
                "pop rdi\n"
                "sub rsi, rdi\n"
                "push rsi");
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

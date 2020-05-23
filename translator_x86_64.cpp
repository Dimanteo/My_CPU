#include "translator_x86_64.h"


int main(int argc, char* argv[])
{
    MODES mode = BINARY;
    char* out_filename = "b.out";
    if (argc > 1)
    {
        if (strcmp(argv[1], "-s") == 0)
        {
            mode = TEXT;
        } else if (argc > 2) {
            printf("Incorrect translator flag: %s\n", argv[1]);
            return 1;
        } else {
            out_filename = argv[1];
        }
        if (argc > 2)
        {
            out_filename = argv[2];
        }
    }

    size_t res_size = 0;
    char* result = translate(BIN_FILE_NAME, mode, &res_size);
    assert(result);
    
    if (mode == TEXT)
    {
        plain_print(out_filename, result);
    } else {
        Exec_Creator creator;

#define EXEC_CREATOR_LOG        
#ifndef NDEBUG
    #define EXEC_CREATOR_LOG , ELF_MAKER_LOG
#endif

        creator.create_exec(Exec_Creator::ELF, result, res_size, out_filename EXEC_CREATOR_LOG);

#undef EXEC_CREATOR_LOG

    }
    free(result);
    return 0;
}


//////////////////// Command_x86_64 properties //////////////////////


Command_x86_64::Command_x86_64(MODES mode) : 
    _mode(mode),
    _body((char*)calloc(CMD_BUFF_SIZE, sizeof(_body[0]))),
    _size(0)
    {}


Command_x86_64::~Command_x86_64() 
{
    free(_body);
    _body = nullptr;
    _size = 0;
}


void Command_x86_64::dump(FILE* log_fp)
{
    fprintf(log_fp, "REGIME : [%d] %s\n"
                  "size   : %d\n"
                  "body\n{\n%s\n}\n",
                  _mode, _mode == TEXT ? "TEXT" : "BINARY",
                  _size, _body);
}


char* Command_x86_64::get_body()
{
    return _body;
}

size_t Command_x86_64::get_size()
{
    if (_mode == BINARY)
    {
        return _size;
    } else {
        return 0;
    }
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

    if (_mode == TEXT)
    {
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
}


int Command_x86_64::translate_cmd(char* src, int pc, size_t offsets[])
{
    assert(src);
    assert(offsets);

    if (_mode == TEXT)
    {
        pc = translate_text(src, pc);
    } else {
#ifndef NDEBUG
        FILE* log_file = fopen(TRANSLATOR_LOG, "a");
        fprintf(log_file, "\nOffset         = %d\n", offsets[pc]);
        fclose(log_file);
#endif        
        pc = translate_bin(src, pc);
    }
    return pc;
}


//////////////////////// Text translation properties //////////////////////////////


void plain_print(const char* filename, const char* text)
{
    assert(filename);
    assert(text);
    FILE* file = fopen(filename, "wb");
    assert(file);
    fwrite(text, sizeof(text[0]), strlen(text), file);
    fclose(file);
}


void make_epilogue(char* dst, MODES mode)
{
    assert(dst);
    size_t size = 0;
    char* stdlib = read_file_to_buffer_alloc(STDTXT_FILENAME, "rb", &size);
    strcat(dst, stdlib);
    free(stdlib);
}


/////////////////////////// Binary translation properties ///////////////////////


// Jump destination address calculates as relative offset from next command.

void patch_on_byte(char* src, int pc, char* dst, size_t offsets[], size_t instr_sz, int byte_pos)
{
    int dst_pc = *(int*)(src + pc + 1);
    uint32_t distance = offsets[dst_pc] - (offsets[pc] + instr_sz);
    size_t arg_offset = offsets[pc] + byte_pos;
    memcpy(dst + arg_offset, &distance, 4);
}


void patch(char* src, int pc, size_t src_size, char* dst, size_t offsets[])
{

    Command_x86_64 command(BINARY);
    for (;pc < src_size; pc++)
    {
        if (offsets[pc] != 0)
        {
            char code = src[pc];
            command.translate_cmd(src, pc, offsets);
            switch (code)
            {
                case CMD_JUMP:
                    patch_on_byte(src, pc, dst, offsets, command.get_size(), 1);
                    break;
                case CMD_JUMPA:
                case CMD_JUMPAE:
                case CMD_JUMPB:
                case CMD_JUMPBE:
                case CMD_JUMPE:
                case CMD_JUMPNE:
                    patch_on_byte(src, pc, dst, offsets, command.get_size(), 7);
                    break;
                case CMD_CALL:
                    patch_on_byte(src, pc, dst, offsets, command.get_size(), 1);
                    break;
            }
        }
    }
}


///////////////////////////////// General translation functions ////////////////////////////////


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


size_t make_prologue(char* dst, MODES mode, size_t offsets[])
{
    assert(dst);
    assert(offsets);

    if (mode == TEXT)
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
    } else {
        /* On start adresses of input and output functions 
           are stored in r13 and r14.
           After that RAM is allocated, x87 is initialized
           and control transfered to user's code. */
        uint8_t init[] = 
        {
            0x49, 0xbd, 0xa6, 0x00, 0x40, 0x00, 0x00,       // mov r13, stdIN
        	0x00, 0x00, 0x00, 
        	0x49, 0xbe, 0x3e, 0x01, 0x40, 0x00, 0x00,       // mov r14. stdOUT
        	0x00, 0x00, 0x00, 
            0x48, 0x81, 0xec, 0x00, 0x00, 0x00, 0x00, 	    // sub rsp, RAM_SIZE
        	0x48, 0x89, 0xe5,             	                // mov rbp, rsp
        	0x9b, 0xdb, 0xe3,                               // finit
        	0xe9, 0x46, 0x01, 0x00, 0x00                    // jmp Main
        };
        size_t init_size = sizeof(init);
        memcpy(init + 23, &RAM_SIZE, 4);                    // insert size of ram in init block

        size_t in_size = 0, out_size = 0;
        char* in_bin = read_file_to_buffer_alloc(STDIN_BINARY, "rb", &in_size);
        char* out_bin = read_file_to_buffer_alloc(STDOUT_BINARY, "rb", &out_size);
        memcpy(dst, init, init_size);
        memcpy(dst + init_size, in_bin, in_size);
        memcpy(dst + init_size + in_size, out_bin, out_size);
        free(in_bin);
        free(out_bin);
        offsets[Command_x86_64::STDIN_POS]  = init_size;
        offsets[Command_x86_64::STDOUT_POS] = init_size + in_size;
        return init_size + in_size + out_size;
    }
    return 0;
}


char* translate(const char* bin_file, MODES mode, size_t* dst_size)
{
    assert(bin_file);
    assert(dst_size);

    *dst_size = 0;
    size_t src_size = 0;
    char* src = read_file_to_buffer_alloc(bin_file, "rb", &src_size);
    size_t offsets[src_size + 1] = {0};

#ifndef NDEBUG
    FILE* log_file = fopen(TRANSLATOR_LOG, "w");
    time_t now = time(nullptr);
    fprintf(log_file, "%s\nMax PC value = %d\n", ctime(&now), src_size - 1);
    fclose(log_file);
#endif

    int pc = check_source(src);
    int entry_p = pc;

    char* dst = (char*)calloc(MAX_PROG_SIZE, sizeof(dst[0]));

    offsets[pc] = make_prologue(dst, mode, offsets);        // put stdIN offset in [0] and stdOUT in [1]

    Command_x86_64 command(mode);
    int previous_offs = 0;
    while (pc < src_size)
    {
        previous_offs = offsets[pc];
        pc = command.translate_cmd(src, pc, offsets);
        if (mode == TEXT)
        {
            strcat(dst, command.get_body());
        } else {
            memcpy(dst + previous_offs, command.get_body(), command.get_size());
        }
        offsets[pc] = previous_offs + command.get_size();
    }
    *dst_size = offsets[src_size];

    if (mode == BINARY)
    {
        patch(src, entry_p, src_size, dst, offsets);
    }
    if (mode == TEXT)
    {
        make_epilogue(dst, mode);
    }

    free(src);
    return dst;
}


#define LBL _body, "%s:\n"
#define GETARGS(n)  nargs = n ;\
                    int args[nargs] = {};\
                    get_args(src + pc, args, nargs);

int Command_x86_64::translate_text(char* src, int pc)
{
    assert(src);

    char label[CMD_BUFF_SIZE] = {0};
    make_label(pc, label);
    char code = src[pc];
    int nargs = -1;
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
                "\tsub rdi, rsi\n"
                "\tpush rdi\n",
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
                "\tfdivr\n"
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
                "\tpush r8\n"
                "\tpush r9\n" 
                "\tpush r10\n" 
                "\tpush r11\n"
                "\tcall stdIN\n"
                "\tpop r11\n"
                "\tpop r10\n"
                "\tpop r9\n"
                "\tpop r8\n"
                "\tpush rax\n",
                label);
            break;
        }
        case CMD_OUT:
        {
            nargs = 0;
            sprintf(LBL 
                "; OUT\n"
                "\tpop rax\n"
                "\tpush r8\n"
                "\tpush r9\n" 
                "\tpush r10\n" 
                "\tpush r11\n"
                "\tpush rax\n"
                "\tcall stdOUT\n"
                "\tpop r11\n"
                "\tpop r10\n"
                "\tpop r9\n"
                "\tpop r8\n",
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
                    strcpy(instr, "jg");
                    break;
                case CMD_JUMPAE:
                    strcpy(instr, "jge");
                    break;
                case CMD_JUMPB:
                    strcpy(instr, "jl");
                    break;
                case CMD_JUMPBE:
                    strcpy(instr, "jle");
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
                "\tmov qword [rbp + 8 * %d], rax\n",
                label,
                args[0]);
            break;
        }
        case CMD_POPRAM:
        {
            GETARGS(1)
            sprintf(LBL
                "; POPRAM\n"
                "\tmov rax, [rbp + 8 * %d]\n"
                "\tpush rax\n",
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
                "; PUSHRAM_X\n"
                "\tmov rax, %s\n"
                "\txor rdx, rdx\n"
                "\tmov rbx, 1000\n"
                "\tdiv rbx\n"
                "\tpop rbx\n"
                "\tmov qword [rbp + 8 * rax], rbx\n",
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
                "; POPRAM_X\n"
                "\tmov rax, %s\n"
                "\txor rdx, rdx\n"
                "\tmov rbx, 1000\n"
                "\tdiv rbx\n"
                "\tmov rax, qword [rbp + 8 * rax]\n"
                "\tpush rax\n",
                label,
                reg);
            break;
        }
        case CMD_POPRAM_NX:
        {
            GETARGS(2)
            char reg[4] = {0};
            match_reg(args[1], reg);
            sprintf(LBL
                "; POPRAM_NX\n"
                "\tmov rax, %s\n"
                "\txor rdx, rdx\n"
                "\tmov rbx, 1000\n"
                "\tdiv rbx\n"
                "\tpop rbx\n"
                "\tmov qword [rbp + 8 * rax + 8 * %d], rbx\n",
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
                "; POPRAM_XN\n"
                "\tmov rax, %s\n"
                "\txor rdx, rdx\n"
                "\tmov rbx, 1000\n"
                "\tdiv rbx\n"
                "\tpop rbx\n"
                "\tmov qword [rbp + 8 * rax + 8 * %d], rbx\n",
                label,
                reg,
                args[1]);
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
                "\txor rdx, rdx\n"
                "\tmov rbx, 1000\n"
                "\tdiv rbx\n"
                "\tmov rax, [rbp + 8 * rax + 8 * %d]\n"
                "\tpush rax\n",
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
                "; PUSHRAM_XN\n"
                "\tmov rax, %s\n"
                "\txor rdx, rdx\n"
                "\tmov rbx, 1000\n"
                "\tdiv rbx\n"
                "\tmov rax, [rbp+ 8 * rax + 8 * %d]\n"
                "\tpush rax\n",
                label,
                reg,
                args[1]);
            break;
        }
        case CMD_POWER:
        {
            nargs = 0;
            sprintf(LBL
                "; POW\n"
                "\tpush 1000\n"
                "\tpush 1\n"
                "\tfild qword [rsp]\n"
                "\tfild qword [rsp+8]\n"
                "\tfild qword [rsp+16]\n"
                "\tfdivr\n"
                "\tfild qword [rsp+8]\n"
                "\tfild qword [rsp+24]\n"
                "\tfdivr\n"
                "\tadd rsp, 24\n"
                "\tfyl2x\n"
                "\tfst qword [rsp]\n"
                "\tfld qword [rsp]\n"
                "\tfrndint\n"
                "\tfxch st1\n"
                "\tfprem\n"
                "\tf2xm1\n"
                "\tfadd st0, st2\n\n"
                "\tfscale\n"
                "\tpush 1000\n"
                "\tfild qword [rsp]\n"
                "\tadd rsp, 8\n"
                "\tfmul\n"
                "\tfistp qword [rsp]\n"
                "\tffree st1\n"
                "\tffree st0\n",
                label);
            break;
        }
    }

    assert(nargs != -1);
    
    pc += 1 + nargs * sizeof(int); 
    return pc;
}

#undef LBL 
#undef GETARGS


#define CLEARCPY\
            _size = sizeof(cmd);\
            memcpy(_body, cmd, _size);
#define GETARGS(n)\
            nargs = n;\
            int argv[2] = {0};\
            get_args(src + pc, argv, nargs);

int Command_x86_64::translate_bin(char* src, int pc)
{
    assert(src);

    char code = src[pc];
    int nargs = -1;

    switch (code)
    {
        case CMD_END:
        {
            nargs = 0;
            uint8_t cmd[] = 
            {
                0xb8, 0x3c, 0x00, 0x00, 0x00,   // mov rax, 0x3C
                0xbf, 0x00, 0x00, 0x00, 0x00,   // mov rdi, 0
                0x0f, 0x05                      // syscall
            };
            CLEARCPY;
            break;
        }
        case CMD_PUSH:
        {
            nargs = 1;
            uint8_t cmd[] =
            {
                0x68, 0x00, 0x00, 0x00, 0x00    // push X
            };
            memcpy(cmd + 1, src + pc + 1, 4);
            CLEARCPY;
            break;
        }
        case CMD_POP:
        {
            GETARGS(1)
            uint8_t reg = 0;
            switch(argv[0])
            {
                case AX:
                    reg = 0x58;
                    break;
                case BX:
                    reg = 0x59;
                    break;
                case CX:
                    reg = 0x5a;
                    break;
                case DX:
                    reg = 0x5b;
                    break;
            }
            uint8_t cmd[] =
            {
                0x41, reg
            };
            CLEARCPY;
            break;
        }
        case CMD_ADD:
        {
            nargs = 0;
            uint8_t cmd[] =
            {
                0x5e,               // pop rsi
                0x5f,               // pop rdi
                0x48, 0x01, 0xfe,   // add rsi, rdi
                0x56                // push rsi
            };
            CLEARCPY;
            break;
        }
        case CMD_SUB:
        {
            nargs = 0;
            uint8_t cmd[] =
            {
                0x5e,               // pop rsi
                0x5f,               // pop rdi
                0x48, 0x29, 0xf7,   // sub rdi, rsi
                0x57                // push rdi
            };
            CLEARCPY;
            break;
        }
        case CMD_MUL:
        {
            nargs = 0;
            uint8_t cmd[] =
            {
                0x68, 0xe8, 0x03, 0x00, 0x00,   // push 1000
                0xdf, 0x6c, 0x24, 0x10,         // fild [rsp+16]
                0xdf, 0x6c, 0x24, 0x08,         // fild [rsp+8]
                0xdf, 0x2c, 0x24,             	// fild [rsp]
                0x48, 0x83, 0xc4, 0x10,         // add rsp, 16
                0xde, 0xf9,                	    // fdiv
                0xde, 0xc9,                	    // fmul
                0xdf, 0x3c, 0x24                // fistp [rsp]
            };
            CLEARCPY;
            break;
        }
        case CMD_DIV:
        {
            nargs = 0;
            uint8_t cmd[] =
            {
                0x68, 0xe8, 0x03, 0x00, 0x00,   // push 1000
                0xdf, 0x2c, 0x24,             	// fild qword [rsp]
                0xdf, 0x6c, 0x24, 0x08,         // fild qword [rsp+8]
                0xdf, 0x6c, 0x24, 0x10,         // fild qword [rsp+16]
                0x48, 0x83, 0xc4, 0x10,         // add rsp, 16
                0xde, 0xf1,                	    // fdiv
                0xde, 0xc9,                	    // fmul
                0xdf, 0x3c, 0x24                // fistp qword [rsp]
            };
            CLEARCPY;
            break;
        }
        case CMD_SQR:
        case CMD_SIN:
        case CMD_COS:
        {
            nargs = 0;
            uint8_t operand = 0;
            switch (code)
            {
                case CMD_SQR:
                    operand = 0xfa;
                    break;
                case CMD_SIN:;
                    operand = 0xfe;
                    break;
                case CMD_COS:
                    operand = 0xff;
                    break;
            }
            uint8_t cmd[] =
            {
                0x68, 0xe8, 0x03, 0x00, 0x00,   // push 1000
                0xdf, 0x2c, 0x24,               // fild qword [rsp]
                0xdf, 0x6c, 0x24, 0x08,         // fild qword [rsp+8]
                0xdf, 0x2c, 0x24,               // fild qword [rsp]
                0x48, 0x83, 0xc4, 0x08,         // add rsp, 8
                0xde, 0xf9,                     // fdiv
                0xd9, operand,                  // fsqrt/fsin/fcos
                0xde, 0xc9,                     // fmul
                0xdf, 0x3c, 0x24                // fistp qword [rsp]
            };
            CLEARCPY;
            break;
        }
/*
push r8
push r9
push r10
push r11
*/        
#define SAVEREGS \
0x41, 0x50, \
0x41, 0x51, \
0x41, 0x52, \
0x41, 0x53
/*
pop r11
pop r10
pop r9
pop r8
*/
#define LOADREGS \
0x41, 0x5b, \
0x41, 0x5a, \
0x41, 0x59, \
0x41, 0x58
        case CMD_IN:
        {
            nargs = 0;
            uint8_t cmd[] =
            {
                SAVEREGS,
                0x41, 0xff, 0xd5,               // call r13 (stdIN)
                LOADREGS,
                0x50                            // push rax
            };
            CLEARCPY;
            break;
        }
        case CMD_OUT:
        {
            nargs = 0;
            uint8_t cmd[] =
            {
                0x58,                           // pop rax
                SAVEREGS,
                0x50,                           // push rax
                0x41, 0xff, 0xd6,               // call r14 (stdOUT)
                LOADREGS
            };
            CLEARCPY;
            break;
        }
#undef SAVEREGS
#undef LOADREGS

        case CMD_PUSHX:
        {
            GETARGS(1);
            uint8_t reg = 0;
            switch(argv[0])
            {
                case AX:
                    reg = 0x50;
                    break;
                case BX:
                    reg = 0x51;
                    break;
                case CX:
                    reg = 0x52;
                    break;
                case DX:
                    reg = 0x53;
                    break;
            }
            uint8_t cmd[] =
            {
                0x41, reg
            };
            CLEARCPY;
            break;
        }
        case CMD_JUMP:
        {
            nargs = 1;
            uint8_t cmd[] =
            {
                0xe9, 0x00, 0x00, 0x00, 0x00    // jmp Address
            };
            CLEARCPY;
            break;
        }
        case CMD_JUMPA:
        case CMD_JUMPAE:
        case CMD_JUMPB:
        case CMD_JUMPBE:
        case CMD_JUMPE:
        case CMD_JUMPNE:
        {
            uint8_t byte = 0;
            switch (code)
            {
                case CMD_JUMPA:
                    byte = 0x8f;
                    break;
                case CMD_JUMPAE:
                    byte = 0x8d;
                    break;
                case CMD_JUMPB:
                    byte = 0x8c;
                    break;
                case CMD_JUMPBE:
                    byte = 0x8e;
                    break;
                case CMD_JUMPE:
                    byte = 0x84;
                    break;
                case CMD_JUMPNE:
                    byte = 0x85;
                    break;
            }
            nargs = 1;
            uint8_t cmd[] =
            {
                0x5f,                                   // pop rdi
                0x5e,                                   // pop rsi
                0x48, 0x39, 0xf7,                       // cmp rdi, rsi
                0x0f, byte, 0x00, 0x00, 0x00, 0x00      // jxx Addr
            };
            CLEARCPY;
            break;
        }
        case CMD_CALL:
        {
            nargs = 1;
            uint8_t cmd[] =
            {
                0xe8, 0x00, 0x00, 0x00, 0x00
            };
            CLEARCPY;
            break;
        }
        case CMD_RET:
        {
            nargs = 0;
            uint8_t cmd[] = {0xc3};
            CLEARCPY;
            break;
        }
        case CMD_PUSHRAM:
        {
            GETARGS(1)
            uint8_t cmd[] =
            {
                0x58,                                       // pop rax
                0x48, 0x89, 0x85, 0x00, 0x00, 0x00, 0x00    // mov qword [rbp + Const], rax
            };
            argv[0] *= 8;
            memcpy(cmd + 4, (uint8_t*)argv, 4);
            CLEARCPY
            break;
        }
        case CMD_POPRAM:
        {
            GETARGS(1)
            uint8_t cmd[] =
            {
                0x48, 0x8b, 0x85, 0x00, 0x00, 0x00, 0x00,   // mov rax, qword [rbp + Const]
                0x50                                        // push rax
            };
            argv[0] *= 8;
            memcpy(cmd + 3, (uint8_t*)argv, 4);
            CLEARCPY
            break;
        }
        case CMD_PUSHRAM_X:
        {
            GETARGS(1)
            uint8_t reg = 0;
            switch(argv[0])
            {
                case AX:
                    reg = 0xc0;
                    break;
                case BX:
                    reg = 0xc8;
                    break;
                case CX:
                    reg = 0xd0;
                    break;
                case DX:
                    reg = 0xd8;
                    break;
            }
            uint8_t cmd[] =
            {
                0x4c, 0x89, reg,                // mov rax, Reg
                0x48, 0x31, 0xd2,               // xor rdx, rdx
                0xbb, 0xe8, 0x03, 0x00, 0x00,   // mov rbx, 1000
                0x48, 0xf7, 0xf3,               // div rbx
                0x5b,                           // pop rbx
                0x48, 0x89, 0x5c, 0xc5, 0x00    // mov qword [rbp + 8 * rax], rbx
            };
            CLEARCPY
            break;
        }
        case CMD_POPRAM_X:
        {
            GETARGS(1)
            uint8_t reg = 0;
            switch(argv[0])
            {
                case AX:
                    reg = 0xc0;
                    break;
                case BX:
                    reg = 0xc8;
                    break;
                case CX:
                    reg = 0xd0;
                    break;
                case DX:
                    reg = 0xd8;
                    break;
            }
            uint8_t cmd[] =
            {
                0x4c, 0x89, reg,                // mov rax, Reg
                0x48, 0x31, 0xd2,               // xor rdx, rdx
                0xbb, 0xe8, 0x03, 0x00, 0x00,   // mov rbx, 1000
                0x48, 0xf7, 0xf3,               // div rbx
                0x48, 0x8b, 0x44, 0xc5, 0x00,   // mov rax, qword [rbp + 8 * rax]
                0x50                            // push rax
            };
            CLEARCPY
            break;
        }
        case CMD_POPRAM_NX:
        case CMD_POPRAM_XN:
        {
            GETARGS(2)
            int number = 0;
            int reg_code = 0;
            uint8_t reg = 0;
            if (code == CMD_PUSHRAM_NX)
            {
                number = argv[0];
                reg_code = argv[1];
            } else {
                number = argv[1];
                reg_code = argv[0];
            }
            switch(reg_code)
            {
                case AX:
                    reg = 0xc0;
                    break;
                case BX:
                    reg = 0xc8;
                    break;
                case CX:
                    reg = 0xd0;
                    break;
                case DX:
                    reg = 0xd8;
                    break;
            }
            uint8_t cmd[] =
            {
                0x4c, 0x89, reg,                                // mov rax, Reg
                0x48, 0x31, 0xd2,                               // xor rdx, rdx
                0xbb, 0xe8, 0x03, 0x00, 0x00,                   // mov rbx, 1000
                0x48, 0xf7, 0xf3,                               // div rbx
                0x5b,                                           // pop rbx
                0x48, 0x89, 0x9c, 0xc5, 0x00, 0x00, 0x00, 0x00  // mov qword [rbp + 8 * rax + Const], rbx
            };
            int mov_arg_offset = 19;
            number *= 8;
            memcpy(cmd + mov_arg_offset, &number, 4);
            CLEARCPY
            break;
        }
        case CMD_PUSHRAM_NX:
        case CMD_PUSHRAM_XN:
        {
            GETARGS(2)
            int number = 0;
            int reg_code = 0;
            uint8_t reg = 0;
            if (code == CMD_PUSHRAM_NX)
            {
                number = argv[0];
                reg_code = argv[1];
            } else {
                number = argv[1];
                reg_code = argv[0];
            }
            switch(reg_code)
            {
                case AX:
                    reg = 0xc0;
                    break;
                case BX:
                    reg = 0xc8;
                    break;
                case CX:
                    reg = 0xd0;
                    break;
                case DX:
                    reg = 0xd8;
                    break;
            }
            uint8_t cmd[] =
            {
                0x4c, 0x89, reg,                                    // mov rax, Reg
                0x48, 0x31, 0xd2,                                   // xor rdx, rdx
                0xbb, 0xe8, 0x03, 0x00, 0x00,                       // mov rbx, 1000
                0x48, 0xf7, 0xf3,                                   // div rbx
                0x48, 0x8b, 0x84, 0xc5, 0x00, 0x00, 0x00, 0x00,     // mov rax, [rbp + 8 * rax + Const]
                0x50                                                // push rax
            };
            int mov_arg_offset = 18;
            number *= 8;
            memcpy(cmd + mov_arg_offset, &number, 4);
            CLEARCPY
            break;
        }
        case CMD_POWER:
        {
            nargs = 0;
            uint8_t cmd[] = 
            {
                0x68, 0xe8, 0x03, 0x00, 0x00,   // push   1000
                0x6a, 0x01,              	    // push   1
                0xdf, 0x2c, 0x24,          	    // fild   QWORD PTR [rsp]
                0xdf, 0x6c, 0x24, 0x08,         // fild   QWORD PTR [rsp+0x8]
                0xdf, 0x6c, 0x24, 0x10,         // fild   QWORD PTR [rsp+0x10]
                0xde, 0xf1,          	        // fdivp  st(1),st
                0xdf, 0x6c, 0x24, 0x08,         // fild   QWORD PTR [rsp+0x8]
                0xdf, 0x6c, 0x24, 0x18,         // fild   QWORD PTR [rsp+0x18]
            	0xde, 0xf1,             	    // fdivp  st(1),st
            	0x48, 0x83, 0xc4, 0x18,         // add    rsp,0x18
            	0xd9, 0xf1,           	        // fyl2x  
            	0xdd, 0x14, 0x24,             	// fst    QWORD PTR [rsp]
            	0xdd, 0x04, 0x24,             	// fld    QWORD PTR [rsp]
            	0xd9, 0xfc,           	        // frndint 
            	0xd9, 0xc9,           	        // fxch   st(1)
            	0xd9, 0xf8,           	        // fprem  
            	0xd9, 0xf0,           	        // f2xm1  
            	0xd8, 0xc2,          	        // fadd   st,st(2)
            	0xd9, 0xfd,           	        // fscale 
            	0x68, 0xe8, 0x03, 0x00, 0x00,   // push   1000
            	0xdf, 0x2c, 0x24,             	// fild   QWORD PTR [rsp]
            	0x48, 0x83, 0xc4, 0x08,         // add    rsp,8
            	0xde, 0xc9,             	    // fmulp  st(1),st
            	0xdf, 0x3c, 0x24,             	// fistp  QWORD PTR [rsp]
            	0xdd, 0xc1,           	        // ffree  st(1)
            	0xdd, 0xc0,           	        // ffree  st(0)
            };
            CLEARCPY
            break;
        }
    }

#ifndef NDEBUG
    FILE* log_file = fopen(TRANSLATOR_LOG, "a");
    fprintf(log_file, "PC             = %d\n"
                      "Code           = %c [%#X]\n"
                      "Number of args = %d\n",
            pc, code, code, nargs);
    switch (code)
    {            
#define DEF_CMD(CMD_name, token, scanf_sample, number_of_args, instructions, disasm_print)\
        case CMD_##CMD_name:\
            fprintf(log_file, #CMD_name "\n");\
            break;

    #include "commands.h"

    }
#undef DEF_CMD
    dump(log_file);
    fclose(log_file);
#endif

    assert(_size);
    assert(nargs != -1);

    pc += 1 + nargs * sizeof(int);
    return pc;
}

#undef GETARGS
#undef CLEARCPY

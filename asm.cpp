#include <ctime>
#include <cmath>
#include "My_Headers/txt_files.h"
#include "linker.h"



int translate_reg(char reg[2]);

int main()
{
    //Парсим на строки
    size_t buf_size = 0, number_lines = 0;
    char* buffer = read_file_to_buffer_alloc("../asm_stdin.txt", "r", &buf_size);
    assert(buffer);
    String* data = parse_buffer_strings_alloc(buffer, &number_lines);
    assert(data);

    //Открываем файл для листинга
    FILE* listing = fopen("../compile.lst", "w");
    assert(listing);
    time_t now = time(nullptr);
    fprintf(listing, "Compilation log: %s\nSIGNATURE: %X\nVERSION: %X\n", ctime(&now), SIGNATURE, VERSION);

    //Создаем массив для записи
    char* bin = (char*)calloc(number_lines * (sizeof(char) + MAX_NUM_ARGS * sizeof(int)), sizeof(char) + sizeof(SIGNATURE) +
            sizeof(VERSION));
    assert(bin);
    char* bin_ptr = bin;
    *(int*)bin_ptr = SIGNATURE;
    bin_ptr += sizeof(SIGNATURE);
    *bin_ptr = VERSION;
    bin_ptr += sizeof(VERSION);

    //транслируем команды
    size_t size_bin = sizeof(SIGNATURE) + sizeof(VERSION);
    for (int pc = 0; pc < number_lines; ++pc)
    {
        char cmd[MAX_CMD_LEN] = {};
        int symb_passed = 0;
        sscanf(data[pc].begin, "%s%n", cmd, &symb_passed);
        char sarg[MAX_REG_LEN + 1] = {};//+1 for \0
        double tmp_crutch = 0;
        double* darg = &tmp_crutch;
        //TODO решить проблему переопределения операторов ассемблера.
        #define DEF_CMD(name, token, scanf_sample, code, n_args, instructions) \
            if((strcmp(#token, cmd) == 0) /*&&*/ scanf_sample)\
            {\
                *bin_ptr = (char)code;\
                fprintf(listing, "%d[%X]", code, code);\
                bin_ptr++;\
                size_bin += sizeof(char) + n_args * sizeof(int);\
                for(int j = 0; j < n_args; j++) \
                {\
                    double farg = 0;\
                    char reg[MAX_REG_LEN + 1] = {};\
                    int tmp_symb_passed = 0;\
                    if(sscanf(data[pc].begin + symb_passed, "%lf%n", &farg, &tmp_symb_passed)) \
                    {\
                        symb_passed += tmp_symb_passed;\
                        *((int*)bin_ptr) = (int)round(farg * 1000);\
                        fprintf(listing, " %d[%.8X]", *((int*)bin_ptr), *((int*)bin_ptr));\
                        bin_ptr += sizeof(int);\
                    } else {\
                        sscanf(data[pc].begin + symb_passed, "%s%n", reg, &tmp_symb_passed);\
                        symb_passed += tmp_symb_passed;\
                        int arg = translate_reg(reg);\
                        *((int*)bin_ptr) = arg;\
                        fprintf(listing, " %d[%.8X]", *((int*)bin_ptr), *((int*)bin_ptr));\
                        bin_ptr += sizeof(int);\
                    }\
                }\
                fprintf(listing,"\n");\
                continue;\
            }
        #include "commands.h"
        #undef DEF_CMD
    }
    realloc(bin, size_bin);
    assert(bin);

    //Записываем массив в файл
    FILE* fbin = fopen("../bin.dak", "wb");
    fwrite(bin, sizeof(char), size_bin, fbin);

    fclose(listing);
    fclose(fbin);
    free(bin);
    free(data);
    free(buffer);
    return 0;
}

int translate_reg(char reg[2])
{
    if(strcmp(reg, "ax") == 0)
    {
        return AX;
    }
    if(strcmp(reg, "bx") == 0)
    {
        return BX;
    }
    if(strcmp(reg, "cx") == 0)
    {
        return CX;
    }
    if(strcmp(reg, "dx") == 0)
    {
        return DX;
    }
    return 0xDEADBEEF;
}



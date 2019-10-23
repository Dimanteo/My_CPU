#include <ctime>
#include <cmath>
#include "My_Headers/txt_files.h"
#include "linker.h"

const char INPUT_FILENAME[] = "../Tests/QEquation.txt";
const size_t MAX_NAMES_COUNT = 1004;

struct Mark {
    int code = -2;
    char name[MAX_NAME_LEN] = {};
};

int translate_str(char* str, int size, Mark names[], int* index = nullptr);

int main()
{
    //Парсим на строки
    size_t buf_size = 0, number_lines = 0;
    char* buffer = read_file_to_buffer_alloc(INPUT_FILENAME, "r", &buf_size);
    assert(buffer);
    String* data = parse_buffer_strings_alloc(buffer, &number_lines);
    assert(data);

    //Открываем файл для листинга
    FILE* listing = fopen("../compile.lst", "w");
    assert(listing);
    time_t now = time(nullptr);
    fprintf(listing, "Compilation log: %s\nSIGNATURE: %X\nVERSION: %X\n", ctime(&now), SIGNATURE, VERSION);
    fprintf(listing, "Translation from file: %s\n", INPUT_FILENAME);

    //Создаем массив для записи
    char* bin = (char*)calloc(number_lines * (sizeof(char) + MAX_NUM_ARGS * sizeof(int)), sizeof(char) + sizeof(SIGNATURE) +
            sizeof(VERSION));
    assert(bin);

    //создаем мэп меток и регистров
    Mark names[MAX_NAMES_COUNT] = {
            {AX, "ax"},
            {BX, "bx"},
            {CX, "cx"},
            {DX, "dx"}
    };
    int names_number = 4;

    //транслируем команды
    size_t size_bin = sizeof(SIGNATURE) + sizeof(VERSION);
    for (int compilation_pass = 1; compilation_pass <= 2; ++compilation_pass)
    {

        fprintf(listing, "\nIteration %d:\n", compilation_pass);
        char* bin_ptr = bin;
        *(int*)bin_ptr = SIGNATURE;
        bin_ptr += sizeof(SIGNATURE);
        *bin_ptr = VERSION;
        bin_ptr += sizeof(VERSION);

        for (int pc = 0; pc < number_lines; ++pc)
        {
            char *comment = strchr(data[pc].begin, ';');
            if (comment != nullptr)
            {
                *comment = '\0';
            }

            char *mark_end = strchr(data[pc].begin, ':');
            if (mark_end != nullptr)
            {
                if (compilation_pass == 1)
                {
                    *mark_end = '\0';
                    size_t mark_len = mark_end - data[pc].begin;
                    char* name = (char *) calloc(mark_len + 1, sizeof(char));//+1 for \0
                    sscanf(data[pc].begin, "%s", name);
                    int index = -1;
                    if (translate_str(name, names_number, names, &index) == -1)
                    {
                        if (index != -1)
                        {
                            names[index].code = bin_ptr - bin;
                        } else {
                            sscanf(data[pc].begin, "%s", names[names_number].name);
                            names[names_number++].code = bin_ptr - bin;
                        }
                    }
                    *mark_end = ':';
                    free(name);
                }
                continue;
            }
            char cmd[MAX_CMD_LEN] = {};
            int symb_passed = 0;
            sscanf(data[pc].begin, "%s%n", cmd, &symb_passed);
            char sarg[MAX_NAME_LEN + 1] = {};//+1 for \0
            double tmp_crutch = 0;
            double *darg = &tmp_crutch;
#define DEF_CMD(name, token, scanf_sample, code, n_args, instructions, disasm) \
            if((strcmp(#token, cmd) == 0) /*&&*/ scanf_sample)\
            {\
                *bin_ptr = (char)code;\
                fprintf(listing, "%d[%X]", code, code);\
                bin_ptr++;\
                if(compilation_pass == 1) size_bin += sizeof(char) + n_args * sizeof(int);\
                for(int j = 0; j < n_args; j++) \
                {\
                    double farg = 0;\
                    char str[MAX_NAME_LEN + 1] = {};\
                    int tmp_symb_passed = 0;\
                    if(sscanf(data[pc].begin + symb_passed, " %lf%n", &farg, &tmp_symb_passed)) \
                    {\
                        symb_passed += tmp_symb_passed;\
                        *((int*)bin_ptr) = (int)round(farg * PRECISION);\
                        fprintf(listing, " %d[%.8X]", *((int*)bin_ptr), *((int*)bin_ptr));\
                        bin_ptr += sizeof(int);\
                    } else {\
                        sscanf(data[pc].begin + symb_passed, " %s%n", str, &tmp_symb_passed);\
                        symb_passed += tmp_symb_passed;\
                        int arg = translate_str(str, names_number, names);\
                        *((int*)bin_ptr) = arg;\
                        fprintf(listing, " %d[%.8X]", *((int*)bin_ptr), *((int*)bin_ptr));\
                        bin_ptr += sizeof(int);\
                    }\
                }\
                fprintf(listing,"\n");\
                continue;\
            }\

#include "commands.h"

#undef DEF_CMD
            fprintf(stderr, "Compilation ERROR. Can't translate operator. Line %d", pc + 1);
            FILE *fbin = fopen(BIN_FILE_NAME, "wb"); //clear bin file in case of error
            fprintf(listing, "\n!!!Compilation error. Can't translate operator. Line %d!!!", pc + 1);
            fclose(listing);
            fclose(fbin);
            free(data);
            free(buffer);
            free(bin);
            abort();
        }
    }
    realloc(bin, size_bin);
    assert(bin);


    //Записываем массив в файл
    FILE* fbin = fopen(BIN_FILE_NAME, "wb");
    fwrite(bin, sizeof(char), size_bin, fbin);

    fclose(listing);
    fclose(fbin);
    free(bin);
    free(data);
    free(buffer);
    return 0;
}

int translate_str(char* str, int size, Mark names[], int* index /*= nullptr*/)
{
    for (int i = 0; i < size; ++i)
    {
        if (strcmp(str, names[i].name) == 0)
        {
            if (index != nullptr)
            {
                *index = i;
            }
            return names[i].code;
        }
    }
    if (index != nullptr)
    {
        *index = -1;
    }
    return -1;
}



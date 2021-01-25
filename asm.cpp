#include <ctime>
#include <cmath>
#include "My_Headers/txt_files.h"
#include "linker.h"
#include "Parser.h"

const size_t MAX_NAMES_COUNT = 1000 + NREGS;

struct Mark {
    int code = -2;
    char name[MAX_NAME_LEN] = "";
};

int translate_str(char* str, int size, Mark names[], int* index = nullptr);

/**
 * Find comment in str.
 * @param str String.
 * @return True if line is comment. Otherwise False.
 */
bool preprocess_comment(char* str);

/**
 * Find and process mark.
 * @param adress Current adress, where mark will be pointing.
 * @param str String to process.
 * @param names Array of marks.
 * @param names_number Number of elements in names.
 * @return 1 if str is mark, 0 if str NOT mark
 */
int preprocess_mark(int adress, char* str, Mark names[], int* names_number, int compilation_pass);

size_t process_RAM(const char token[], char *str, char **bin_ptr, FILE *listing);

int main(int argc, char* argv[])
{
    const char* input_filename = nullptr;
    if (argc > 1)
    {
        input_filename = argv[1];
    } else {
        input_filename = INPUT_FILENAME;
    }
    //Парсим на строки
    size_t buf_size = 0, number_lines = 0;
    char* buffer = read_file_to_buffer_alloc(input_filename, "r", &buf_size);
    assert(buffer);
    String* data = parse_buffer_strings_alloc(buffer, &number_lines);
    assert(data);

    //Открываем файл для листинга
    FILE* listing = fopen(LISTING_FILE, "w");
    assert(listing);
    time_t now = time(nullptr);
    fprintf(listing, "Compilation log: %s\nSIGNATURE: %X\nVERSION: %X\n", ctime(&now), SIGNATURE, VERSION);
    fprintf(listing, "Translation from file: %s\n", input_filename);

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
    int names_number = NREGS;

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
            if (preprocess_comment(data[pc].begin))
            {
                continue;
            }
            if(preprocess_mark(bin_ptr - bin, data[pc].begin, names, &names_number, compilation_pass))
            {
                continue;
            }
            char cmd[MAX_CMD_LEN] = "";
            int symb_passed = 0;
            sscanf(data[pc].begin, " %s%n", cmd, &symb_passed);
            char sarg[MAX_NAME_LEN + 1] = "";//+1 for \0
            double tmp_crutch = 0;
            double *darg = &tmp_crutch;
#define DEF_CMD(name, token, scanf_sample, n_args, instructions, disasm) \
            if((strcmp(#token, cmd) == 0) /*&&*/ scanf_sample)\
            {\
                if (char* ram_sep = strchr(data[pc].begin, '[') )\
                {\
                    if (compilation_pass == 1)\
                        size_bin += process_RAM(#token, ram_sep, &bin_ptr, listing);\
                    else\
                        process_RAM(#token, ram_sep, &bin_ptr, listing);\
                    fprintf(listing, "\n");\
                    continue;\
                }\
                *bin_ptr = (char)CMD_##name;\
                fprintf(listing, "%d[%X]", *bin_ptr, *bin_ptr);\
                bin_ptr++;\
                if(compilation_pass == 1) size_bin += sizeof(char) + n_args * sizeof(int);\
                for(int i = 0; i < n_args; i++) \
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
    fclose(fbin);

    fclose(listing);

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

bool preprocess_comment(char* str)
{
    char *comment = strchr(str, ';');
    if (comment != nullptr)
    {
        *comment = '\0';
    }
    bool isblank = true;
    for (;*str; str++)
    {
        if (!isspace(*str))
        {
            isblank = false;
        }
    }
    return isblank;
}

int preprocess_mark(int adress, char* str, Mark names[], int* names_number, int compilation_pass)
{
    char *mark_end = strchr(str, ':');
    if (mark_end != nullptr)
    {
        if (compilation_pass == 1)
        {
            *mark_end = '\0';
            size_t mark_len = mark_end - str;
            char *name = (char *) calloc(mark_len + 1, sizeof(char));//+1 for \0
            sscanf(str, " %s", name);
            int index = -1;
            if (translate_str(name, *names_number, names, &index) == -1)
            {
                if (index != -1) {
                    names[index].code = adress;
                } else {
                    sscanf(str, " %s", names[*names_number].name);
                    names[*names_number].code = adress;
                    *names_number += 1;
                }
            }
            free(name);
            *mark_end = ':';
        }
        return 1;
    }
    return 0;
}

size_t process_RAM(const char token[], char *str, char **bin_ptr, FILE *listing) {
    Parser parser;
    parser.str = str;
    parser.getG();
    if (parser.arg_num == 1)
    {
        if (parser.arg_type[0] == 'n')
        {
            if (strcmp(token, "push") == 0)
                **bin_ptr = (char)CMD_PUSHRAM;
            else
                **bin_ptr = (char)CMD_POPRAM;
        } else {
            if (strcmp(token, "push") == 0)
                **bin_ptr = (char)CMD_PUSHRAM_X;
            else
                **bin_ptr = (char)CMD_POPRAM_X;
        }
    } else if (parser.arg_num == 2) {
        if (parser.arg_type[0] == 'n')
        {
            if (strcmp(token, "push") == 0)
                **bin_ptr = (char)CMD_PUSHRAM_NX;
            else
                **bin_ptr = (char)CMD_POPRAM_NX;
        } else {
            if (strcmp(token, "push") == 0)
                **bin_ptr = (char)CMD_PUSHRAM_XN;
            else
                **bin_ptr = (char)CMD_POPRAM_XN;
        }
    }
    fprintf(listing, "%d[%X]", **bin_ptr, **bin_ptr);
    (*bin_ptr)++;
    for (int i = 0; i  < parser.arg_num; i++)
    {
        *((int*)(*bin_ptr)) = parser.arg_v[i];
        fprintf(listing, " %d[%.8X]", *((int*)(*bin_ptr)), *((int*)(*bin_ptr)));
        *bin_ptr += sizeof(int);
    }
    return sizeof(char) + sizeof(int) * parser.arg_num;
}



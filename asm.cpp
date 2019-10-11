#include <cmath>
#include <ctime>
#include "txt_files.h"
#include "linker.h"


int main()
{
    size_t buf_size = 0, number_lines = 0;
    char* buffer = read_file_to_buffer_alloc("../asm_input.txt", "r", &buf_size);
    assert(buffer);
    String* data = parse_buffer_strings_alloc(buffer, &number_lines);
    assert(data);

    int cmd_arg_counter = 0;
    for (int i = 0; i < number_lines; ++i)
    {
        char cmd[MAX_COMMAND_LEN] = {};
        sscanf(data[i].begin, "%s", cmd);
        assert(cmd);
        for (int j = CMD_CODE_VALUES_BEGIN; j <= CMD_CODE_VALUES_END; ++j) {
            if (!strcmp(cmd, cmd_list[j].str))
            {
                cmd_arg_counter += cmd_list[j].args_n; //все эти заморочки нужны для того чтобы дегко добавлять новые команды
            }
        }
    }

    size_t bin_size = number_lines + cmd_arg_counter * sizeof(int) + sizeof(VERSION) + sizeof(SIGNATURE);
    char* bin = (char*)calloc(bin_size, sizeof(char));
    assert(bin);
    FILE* listing = fopen("../compile.lst", "w");
    assert(listing);
    time_t now = time(nullptr);
    fprintf(listing, "Compilation log: %s\nSIGNATURE: %#X\nVERSION: %d\n", ctime(&now), SIGNATURE, VERSION);
    char* pw = bin;
    *((unsigned int*)pw) = SIGNATURE;
    pw += sizeof(SIGNATURE);
    *pw = VERSION;
    pw++;

    for (int i = 0; i < number_lines; ++i)
    {
        char cmd[MAX_COMMAND_LEN] = {};
        int prefix = 0, args_n = 0;
        sscanf(data[i].begin, "%s%n", cmd, &prefix);

        for (int j = CMD_CODE_VALUES_BEGIN; j <= CMD_CODE_VALUES_END; ++j)
        {
            if(!strcmp(cmd, cmd_list[j].str))
            {
                fprintf(listing, "%x ",(char)cmd_list[j].code);
                *pw = (char)cmd_list[j].code;
                pw++;
                args_n = cmd_list[j].args_n;
                break;
            }
        }

        for (int j = 0; j < args_n; ++j)
        {
            double farg = NAN;
            char sarg[MAX_ARG_LEN] = {};
            int readen = 0;
            if(sscanf(data[i].begin + prefix, "%lf%n", &farg, &readen) == 1)
            {
                *(int*)pw = (int)round(farg * 1000);
                fprintf(listing, "[%#x]%d", *(int*)pw, *(int*)pw);
                pw += sizeof(int);
                prefix += readen;
            } else {
                sscanf(data[i].begin + prefix, "%s%n", sarg, &readen);
                if (!strcmp(sarg, "ax"))
                {
                    *(int *)pw = AX;
                    pw += sizeof(REG_CODE);
                    prefix += readen;
                }
                else if (!strcmp(sarg, "bx"))
                {
                    *pw = (char)BX;
                    pw += sizeof(REG_CODE);
                    prefix += readen;
                }
                else if (!strcmp(sarg, "cx"))
                {
                    *pw = (char)CX;
                    pw += sizeof(REG_CODE);
                    prefix += readen;
                }
                else if (!strcmp(sarg, "dx"))
                {
                    *pw = (char)DX;
                    pw += sizeof(REG_CODE);
                    prefix += readen;
                }
                fprintf(listing, "[%#x]%d", *(pw - 1), *(pw - 1));
            }
            fprintf(listing, "\n");
        }
    }

    FILE* fbin = fopen("../bin.dak", "wb");
    fwrite(bin, sizeof(char), bin_size, fbin);

    fclose(listing);
    fclose(fbin);
    free(bin);
    free(data);
    free(buffer);
    return 0;
}


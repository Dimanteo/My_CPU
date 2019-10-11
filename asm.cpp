#include <cmath>
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

    char* bin = (char*)calloc(number_lines + cmd_arg_counter * sizeof(int) + sizeof(VERSION) + sizeof(SIGNATURE), sizeof(char));
    assert(bin);
    char* pw = bin;
    ((unsigned int*)pw)[0] = SIGNATURE;
    (unsigned int*)pw++;
    *pw = VERSION;
    for (int i = 0; i < number_lines; ++i)
    {
        char cmd[MAX_COMMAND_LEN] = {};
        int prefix = 0, args_n = 0;
        sscanf(data[i].begin, "%s%n", cmd, &prefix);
        for (int j = CMD_CODE_VALUES_BEGIN; j <= CMD_CODE_VALUES_END; ++j)
        {
            if(!strcmp(cmd, cmd_list[j].str))
            {
                *pw = cmd_list[j].code;
                pw++;
                args_n = cmd_list[j].args_n;
            }
        }
        for (int j = 0; j < args_n; ++j) {
            double far = NAN;
            char sar[MAX_ARG_LEN] = {};
            int readen = 0;
            if(sscanf(data[i].begin + prefix, "%lf%n", &far, &readen) == 1)
            {
                //TODO добавление float
            } else if (sscanf(data[i].begin + prefix, "%s%n", sar, &readen) == 1) {
                //TODO добавление строкового аргумента
            }

        }
    }

    free(bin);
    free(data);
    free(buffer);
    return 0;
}
#include <string.h>
#include <time.h>
#include "linker.h"
#include "My_Headers/txt_files.h"
#include "My_Headers/elf_maker.h"

enum REGIMES {TEXT = 's', BINARY = 0};

/**
 *Contains binary and text representation for single x86_64 intstruction.
 **/
class Command_x86_64
{
private:
    char*     _body;
    size_t    _size;
    REGIMES   _regime;

    void make_label(int pc, char* label);
    void get_args(char* operand, int* buff, int nargs);
    void match_reg(int code, char* reg);

public:
    const static int STDIN_POS  = 0;
    const static int STDOUT_POS = 1;

    Command_x86_64(REGIMES regime);
    int translate_cmd(char* src, int pc, size_t offsets[]);
    char* get_body();
    size_t get_size();
    ~Command_x86_64();
};

int check_source(char* buffer);

size_t make_prologue(char* dst, REGIMES regime, size_t offsets[]);

void make_epilogue(char* dst, REGIMES regime);

char* translate(const char* bin_file, REGIMES regime);

void plain_print(const char* filename, const char* text);

void make_elf(const char* filename, const char* body);

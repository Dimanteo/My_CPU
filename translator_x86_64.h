#include <string.h>
#include "linker.h"
#include "My_Headers/txt_files.h"

enum REGIMES {TEXT = 's', BINARY = 0};

/**
 *Contains binary and text representation for single x86_64 intstruction.
 **/
class Command_x86_64
{
private:
    char*     _body;
    int       _size;
    REGIMES   _regime;

    void make_label(int pc, char* label);
    void get_args(char* operand, int* buff, int nargs);
    void match_reg(int code, char* reg);

public:
    Command_x86_64(REGIMES regime);
    int translate_single(char* src, int pc);
    char* get_body();
    ~Command_x86_64();
};

char* translate(const char* bin_file, REGIMES regime);

void plain_print(const char* filename, const char* text);

void make_elf(const char* filename, const char* body);

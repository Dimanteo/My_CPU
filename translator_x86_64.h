#include "linker.h"

enum REGIMES {TEXT = 's', BINARY = 0};
/**
 *Contains binary and text representation for single x86_64 intstruction.
 **/
class Command_x86_64
{
private:
    char*     _opcodes;
    char*     _binary;
    REGIMES   _regime;

public:
    Command_x86_64(REGIMES regime, char* opcodes, char* binary);
    char* getBody();
    ~Command_x86_64();
};

Command_x86_64::Command_x86_64(REGIMES regime, char* opcodes, char* binary) : 
    _regime  (regime),
    _opcodes (opcodes),
    _binary  (binary) 
    {}

char* Command_x86_64::getBody()
{
    if (_regime == TEXT)
    {
        return _opcodes;
    } else {
        return _binary;
    }
}

Command_x86_64::~Command_x86_64() 
{
    _opcodes = nullptr;
    _binary = nullptr;
}

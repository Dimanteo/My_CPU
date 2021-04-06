#include "../linker.h"
#include <vector>

class Core;

class Insn
{
    CMD_CODE m_code;
    bool m_isBranch;
    int m_argc;
    std::vector<int> m_argv;
    std::string opcode;

    using ExecFunc = void (*)(Core *core);
    ExecFunc m_exec;

    public:
        void exec(Core *core);
        static void decode(const char *pc);
};
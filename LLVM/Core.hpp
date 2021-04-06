#include <vector>
#include "../linker.h"

using word_t = int;
using reg_t = int;
using address_t = size_t;

class Core
{
        std::vector<word_t> m_stack;
        reg_t regFile[NREGS];
        word_t *m_memory;
        bool m_running;

    public:
        Core();
        ~Core();
        void run_core(const char *entry);
        void push(word_t val);
        word_t pop();
        reg_t getReg(REG_CODE regi) const;
        void setReg(REG_CODE regi);
        void read(address_t adr) const;
        void write(address_t adr);
        void stop_core();
};
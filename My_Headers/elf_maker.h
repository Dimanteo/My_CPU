#pragma once
#include <elf.h>
#include <stdlib.h>
#include <stdio.h>


class Exec_Creator 
{
public:    

    enum FORMAT {ELF = 1, EXE = 2};

    Exec_Creator(){}
    ~Exec_Creator(){}

    void create_exec(FORMAT format, const char* body, size_t size, const char* filename, const char* logf = nullptr)
    {
        assert(filename);
        assert(body);

        if (format == ELF)
        {
            char* header = create_elf_head(size);
            create_elf(header, body, size, filename);
            free(header);
        }
#ifndef NDEBUG
        dump(filename, logf);
#endif  
    }

    void dump(const char* elf_file, const char* log_file = nullptr)
    {
        char name_buff[strlen(elf_file) + strlen(".log")] = "";
        if (log_file == nullptr)
        {
            sprintf(name_buff, "%s.log", elf_file);
            log_file = name_buff;
        }
        char request[strlen(log_file) * 3 + strlen(elf_file) * 3 + 61] = "";
        sprintf(request, "readelf -h %s >  %s\n"
                         "readelf -l %s >> %s\n"
                         "objdump -d -M intel %s >> %s\n",
                         elf_file, log_file, 
                         elf_file, log_file,
                         elf_file, log_file);
        system(request);
    }

private:

    const size_t elf_header_size = 0x80;

    char* create_elf_head(uint64_t size)
    {
        // Params for executable ELF header on Linux x86_64
        Elf64_Addr entry_point = 0x400080;
        Elf64_Off phoff = 0x40;
        Elf64_Half ehsize = 0x40;
        Elf64_Half phsize = 56;
        Elf64_Half	phnum = 1;

        Elf64_Ehdr elf_h = 
        {
            {                       // e_ident
                ELFMAG0,
                ELFMAG1,
                ELFMAG2, 
                ELFMAG3, 
                ELFCLASS64, 
                ELFDATA2LSB, 
                EV_CURRENT, 
                ELFOSABI_NONE,
                EI_PAD
            },
            ET_EXEC,                // e_type
            EM_X86_64,              // e_machine
            EV_CURRENT,             // e_version
            entry_point,            // e_entry
            phoff,                  // e_phoff
            0,                      // e_shoff
            0,                      // e_flags
            ehsize,                 // e_ehsize
            phsize,                 // e_phentsize
            phnum,                  // e_phnum
            0,                      // e_shentsize
            0,                      // e_shum
            0                       // e_shstrndx
        };

        // Params for Programm header
        Elf64_Addr address = 0x400000;
        Elf64_Xword align  = 0x200000;

        Elf64_Phdr prog_h = 
        {
            PT_LOAD,                // p_type
            PF_X | PF_R,            // p_flags
            0,  	                // p_offset
            address,	            // p_vaddr
            address,	            // p_paddr
            size,	                // p_filesz
            size,                   // p_memsz
            align	                // p_align
        };

        char* buffer = (char*)calloc(elf_header_size, sizeof(buffer[0]));
        memcpy(buffer, &elf_h, sizeof(elf_h));
        memcpy(buffer + sizeof(elf_h), &prog_h, sizeof(prog_h));
        return buffer;
    }


    void create_elf(const char* header, const char* body, size_t size, const char* filename)
    {
        assert(filename);
        assert(header);
        assert(body);

        FILE* file = fopen(filename, "wb");
        fwrite(header, sizeof(header[0]), elf_header_size, file);
        fwrite(body, sizeof(body[0]), size, file);
        fclose(file);      
    }
};

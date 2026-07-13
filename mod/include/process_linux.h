#pragma once
#ifdef __linux__
#include <string>
#include <vector>
#include "util.h"

#include <elf.h>

struct ImportedFunction
{
    Elf64_Xword reloc_info;
    std::string name;
    Elf64_Addr *addr;
};

class ProcessInterface
{
private:
    Elf64_Addr game_handle;

    Elf64_Ehdr *ehdr = nullptr;
    Elf64_Phdr *phdrs = nullptr;
    Elf64_Dyn *dynamic = nullptr;

    Elf64_Sym *sym_table = nullptr;
    char *str_table = nullptr;
    Elf64_Rela *reloc_table = nullptr;
    Elf64_Xword rel_table_sz = 0;

    std::vector<ImportedFunction> imports;

public:
    Elf64_Addr game_base() const;
    Elf64_Addr offset(const void* a) const;
    
public:
    const std::vector<ImportedFunction> &import_view() const;
    bool find_import(const std::string& symbol, ImportedFunction* out) const;

private:
    Elf64_Addr main_module_base() const;
    void init_game_handle();

public:
    void init();
};
#else
UNSUPPORTED_OS();
#endif
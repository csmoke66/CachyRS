#pragma once
#ifdef __linux__
#include <string>
#include <vector>
#include "util.h"

#include <elf.h>

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
    uint64_t game_base() const;
    uint64_t offset(const void* a) const;
    
public:
    const std::vector<ImportedFunction> &import_view() const;
    bool find_import(const std::string& symbol, ImportedFunction* out) const;

private:
    uint64_t main_module_base() const;
    void init_game_handle();

public:
    void init();
};
#else
UNSUPPORTED_OS();
#endif
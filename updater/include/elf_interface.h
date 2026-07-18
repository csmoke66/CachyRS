#pragma once
#include <elf.h>
#include <string>
#include <vector>

struct ImportedFunction
{
    ::std::string name;
    uint64_t *addr;
};

class ElfInterface
{
private:
    Elf64_Addr elf_data;

    Elf64_Ehdr *ehdr = nullptr;
    Elf64_Phdr *phdrs = nullptr;
    Elf64_Dyn *dynamic = nullptr;

    Elf64_Sym *sym_table = nullptr;
    char *str_table = nullptr;
    Elf64_Rela *reloc_table = nullptr;
    Elf64_Xword rel_table_sz = 0;

    ::std::vector<ImportedFunction> imports;

    ElfInterface(Elf64_Addr data);

public:
    Elf64_Addr offset(const void *a) const;
    const std::vector<ImportedFunction> &import_view() const;

public:
    bool find_import(const std::string &symbol, ImportedFunction *out) const;

public:
    Elf64_Addr ptr_to_va(uint64_t pa) const;
    Elf64_Addr va_to_ptr(uint64_t va) const;

public:
    void init_base();
    void init();
};

#include "process.h"

#ifdef __linux__
#include <limits.h>
#include <unistd.h>

#include <fstream>
#include <sstream>

Elf64_Addr ProcessInterface::game_base() const
{
    return game_handle;
}

Elf64_Addr ProcessInterface::offset(const void *a) const
{
    return (Elf64_Addr)a - game_base();
}

const std::vector<ImportedFunction> &ProcessInterface::import_view() const
{
    return imports;
}

bool ProcessInterface::find_import(const std::string &symbol, ImportedFunction *out) const
{
    for (auto &i : imports)
    {
        if (!i.name.compare(symbol))
        {
            *out = i;
            return true;
        }
    }

    *out = {};
    return false;
}

Elf64_Addr ProcessInterface::main_module_base() const
{
    char exe[PATH_MAX];
    ssize_t len = readlink("/proc/self/exe", exe, sizeof(exe) - 1);
    exe[len] = '\0';

    std::ifstream maps("/proc/self/maps");
    std::string line;

    while (std::getline(maps, line))
    {
        if (line.find(exe) == std::string::npos)
        {
            continue;
        }

        std::istringstream iss(line);

        std::string range;
        std::string perms;
        std::string offset;

        iss >> range >> perms >> offset;

        if (offset != "00000000")
        {
            continue;
        }

        auto dash = range.find('-');
        return std::stoull(range.substr(0, dash), nullptr, 16);
    }

    return 0;
}

void ProcessInterface::init_game_handle()
{
    game_handle = main_module_base();
    if (!game_handle)
    {
        throw std::runtime_error("failed to find game handle");
    }
}

void ProcessInterface::init()
{
    init_game_handle();

    ehdr = (Elf64_Ehdr *)(game_handle);
    phdrs = (Elf64_Phdr *)(game_handle + ehdr->e_phoff);

    for (int i = 0; i < ehdr->e_phnum; i++)
    {
        if (phdrs[i].p_type == PT_DYNAMIC)
        {
            dynamic = (Elf64_Dyn *)(game_handle + phdrs[i].p_vaddr);
            break;
        }
    }

    for (Elf64_Dyn *dyn = dynamic; dyn->d_tag != DT_NULL; dyn++)
    {
        switch (dyn->d_tag)
        {
        case DT_SYMTAB:
            sym_table = (Elf64_Sym *)(dyn->d_un.d_ptr);
            break;

        case DT_STRTAB:
            str_table = (char *)(dyn->d_un.d_ptr);
            break;

        case DT_JMPREL:
            reloc_table = (Elf64_Rela *)(dyn->d_un.d_ptr);
            break;

        case DT_PLTRELSZ:
            rel_table_sz = dyn->d_un.d_val;
            break;
        }
    }

    auto count = rel_table_sz / sizeof(Elf64_Rela);
    for (auto i = 0; i < count; i++)
    {
        auto rela = reloc_table[i];
        auto sym_idx = ELF64_R_SYM(rela.r_info);
        auto &sym = sym_table[sym_idx];
        auto name = &str_table[sym.st_name];
        imports.push_back(ImportedFunction{
            rela.r_info,
            std::string(name),
            (Elf64_Addr *)(game_handle + rela.r_offset)});
    }
}
#endif
#include "process.h"

#ifdef __linux__
#include <limits.h>
#include <unistd.h>
#include <link.h>
#include <climits>

#include <fstream>
#include <sstream>

namespace crs
{
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
        struct CallbackData
        {
            Elf64_Addr base_address = 0;
        } data;

        // clang-format off
        dl_iterate_phdr([](struct dl_phdr_info *info, size_t size, void *data_ptr) -> int
        {
            auto res = (CallbackData *)data_ptr;

            if (info->dlpi_name != nullptr && info->dlpi_name[0] == '\0')
            {
                res->base_address = info->dlpi_addr;
                return 1;
            }

            return 0;
        }, &data);
        // clang-format on
        return data.base_address;
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
                std::string(name),
                (uint64_t *)(game_handle + rela.r_offset)});
        }
    }
}
#endif
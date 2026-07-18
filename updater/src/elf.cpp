#include "elf_interface.h"

ElfInterface::ElfInterface(Elf64_Addr data)
{
    this->elf_data = data;
}

Elf64_Addr ElfInterface::offset(const void *a) const
{
    return (Elf64_Addr)a - elf_data;
}

const std::vector<ImportedFunction> &ElfInterface::import_view() const
{
    return imports;
}

bool ElfInterface::find_import(const std::string &symbol, ImportedFunction *out) const
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

void ElfInterface::init_base()
{
    ehdr = (Elf64_Ehdr *)(elf_data);
    phdrs = (Elf64_Phdr *)(elf_data + ehdr->e_phoff);

    for (int i = 0; i < ehdr->e_phnum; i++)
    {
        if (phdrs[i].p_type == PT_DYNAMIC)
        {
            dynamic = (Elf64_Dyn *)(elf_data + phdrs[i].p_offset);
            break;
        }
    }
}

Elf64_Addr ElfInterface::ptr_to_va(uint64_t pa) const
{
   for (int i = 0; i < ehdr->e_phnum; i++)
    {
        auto &ph = phdrs[i];

        if (ph.p_type != PT_LOAD)
            continue;

        if (pa >= ph.p_offset &&
            pa < ph.p_offset + ph.p_filesz)
        {
            return ph.p_vaddr + (pa - ph.p_offset);
        }
    }

    return 0;
}

Elf64_Addr ElfInterface::va_to_ptr(uint64_t va) const
{
    for (int i = 0; i < ehdr->e_phnum; i++)
    {
        auto &ph = phdrs[i];

        if (ph.p_type != PT_LOAD)
            continue;

        if (va >= ph.p_vaddr &&
            va < ph.p_vaddr + ph.p_memsz)
        {
            auto file_offset = ph.p_offset + (va - ph.p_vaddr);
            return elf_data + file_offset;
        }
    }

    return 0;
}

void ElfInterface::init()
{

    for (Elf64_Dyn *dyn = dynamic; dyn->d_tag != DT_NULL; dyn++)
    {
        switch (dyn->d_tag)
        {
        case DT_SYMTAB:
            sym_table = (Elf64_Sym *)va_to_ptr(dyn->d_un.d_ptr);
            break;

        case DT_STRTAB:
            str_table = (char *)va_to_ptr(dyn->d_un.d_ptr);
            break;

        case DT_JMPREL:
            reloc_table = (Elf64_Rela *)va_to_ptr(dyn->d_un.d_ptr);
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
            (uint64_t *)(rela.r_offset)});
    }
}
#include "pattern.h"
#include "updater.h"
#include <cstring>
#include <iostream>

AlignmentValidator::AlignmentValidator(uint64_t alignment)
{
    this->alignment = alignment;
}

bool AlignmentValidator::validate(const uint64_t *t)
{
    return (*t % alignment) == 0;
}

ImmExtractor::ImmExtractor(uint64_t offset_to_data, uint64_t offset, size_t data_size, bool factor_in_rip)
{
    this->offset_to_data = offset_to_data;
    this->offset = offset;
    this->data_size = data_size;
    this->factor_in_rip = factor_in_rip;
}

uint64_t ImmExtractor::extract(const ElfInterface &elf, const uint8_t *data)
{
    auto data_ptr = data + offset_to_data;
    uint64_t v = 0;

    if (data_size == 1)
    {
        v = *((int8_t *)(data_ptr));
    }
    else if (data_size == 2)
    {
        v = *((int16_t *)(data_ptr));
    }
    else if (data_size == 4)
    {
        v = *((int32_t *)(data_ptr));
    }
    else if (data_size == 8)
    {
        v = *((int64_t *)(data_ptr));
    }

    if (factor_in_rip)
    {
        v += elf.ptr_to_va(elf.offset(data));
    }

    return v + offset;
}

DirectExtractor::DirectExtractor(uint64_t offset)
{
    this->offset = offset;
}

uint64_t DirectExtractor::extract(const ElfInterface &elf, const uint8_t *data)
{
    return elf.ptr_to_va(elf.offset(data));
}

MenuActionHandlerExtractor::MenuActionHandlerExtractor(csh capstone_handle, uint64_t lea_offset)
{
    this->capstone_handle = capstone_handle;
    this->lea_offset = lea_offset;
}

uint64_t MenuActionHandlerExtractor::extract(const ElfInterface &elf, const uint8_t *data)
{
    uint64_t lea_count = 0;

    size_t count;
    cs_insn *insn;

    auto ret = false;
    auto rva = elf.ptr_to_va(elf.offset(data));

    while (!ret)
    {
        count = cs_disasm(capstone_handle, (const uint8_t *)data, 0x15, (uint64_t)rva, 0, &insn);

        if (insn->id == X86_INS_LEA)
        {
            auto x86 = &(insn->detail->x86);
            auto mem = x86->operands[1].mem;

            auto rip = rva + mem.disp + 7;
            std::cout << "LEA: 0x" << std::hex << rip << "@" << std::dec << lea_count << std::endl;
            if (lea_count == lea_offset)
            {
                return rip;
            }

            lea_count += 1;
        }
        else if (insn->id == X86_INS_RET)
        {
            ret = true;
        }

        data += insn->size;
        rva += insn->size;
    }

    return 0;
}

ConstructorSizeExtractor::ConstructorSizeExtractor(csh capstone_handle, x86_reg reg, uint32_t end)
{
    this->capstone_handle = capstone_handle;
    this->reg = reg;
    this->end = end;
}

uint64_t ConstructorSizeExtractor::extract(const ElfInterface &elf, const uint8_t *data)
{
    size_t count;
    cs_insn *insn;

    size_t count_call_iat;
    cs_insn *insn_call_iat;

    auto ret = false;
    uint64_t last_written = 0;
    uint8_t last_sz = 0;

    auto rva = elf.ptr_to_va(elf.offset(data));

    ImportedFunction memset_import;
    elf.find_import("memset", &memset_import);

    while (!ret)
    {
        count = cs_disasm(capstone_handle, (const uint8_t *)data, 0x15, rva, 1, &insn);

        auto x86 = &(insn->detail->x86);
        if (count == 1 && insn->id == X86_INS_MOV)
        {
            if (x86->operands[0].type == x86_op_type::X86_OP_MEM)
            {
                auto op = x86->operands[0];
                auto mem = op.mem;
                if (mem.base == reg)
                {
                    if (mem.disp > last_written)
                    {
                        last_written = mem.disp;
                        last_sz = op.size;
                    }
                }
            }
        }
        else if (insn->id == end)
        {
            ret = true;
        }

        data += insn->size;
        rva += insn->size;

        cs_free(insn, 1);
    }

    return last_written + last_sz;
}

GenericExtractor::GenericExtractor(void *extractor)
{
    this->nested = (Extractor<uint64_t> *)extractor;
}

uint64_t GenericExtractor::extract(const ElfInterface &elf, const uint8_t *data)
{
    return nested->extract(elf, data);
}

DummyExtractor::DummyExtractor(uint64_t v)
{
    this->v = v;
}

uint64_t DummyExtractor::extract(const ElfInterface &elf, const uint8_t *data)
{
    return v;
}

Pattern::Pattern(std::string name, Type type, Extractor<uint64_t> *extractor)
{
    this->name = name;
    this->type = type;
    this->extractor = extractor;
}

DefaultPattern::DefaultPattern(std::string name, std::vector<int> pattern, Type type, Extractor<uint64_t> *extractor) : Pattern(name, type, extractor)
{
    this->pattern = pattern;
}

const uint8_t *DefaultPattern::find_result(uint8_t *text, Elf64_Shdr text_hdr)
{
    const uint8_t *found;
    pattern_scan(text, text_hdr.sh_size, pattern, &found);
    return found;
}

DummyPattern::DummyPattern(std::string name, Type type, Extractor<uint64_t> *extractor) : Pattern(name, type, extractor)
{
}

const uint8_t *DummyPattern::find_result(uint8_t *text, Elf64_Shdr text_hdr)
{
    return nullptr;
}

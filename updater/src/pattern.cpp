#include "pattern.h"
#include "updater.h"
#include <cstring>
#include <iostream>

AlignmentValidator::AlignmentValidator(uint64_t alignment) : alignment(alignment)
{
}

bool AlignmentValidator::validate(const uint64_t *t)
{
  return (*t % alignment) == 0;
}

ImmExtractor::ImmExtractor(uint64_t offset_to_data, uint64_t offset, size_t data_size, bool factor_in_rip) : offset_to_data(offset_to_data),
                                                                                                             offset(offset),
                                                                                                             data_size(data_size),
                                                                                                             factor_in_rip(factor_in_rip)
{
}

uint64_t ImmExtractor::extract(const ElfInterface &elf, const uint8_t *data)
{
  if (!data)
  {
    return 0;
  }

  auto data_ptr = data + offset_to_data;
  uint64_t v = 0;

  if (data_size == 1)
  {
    v = *reinterpret_cast<const int8_t *>(data_ptr);
  }
  else if (data_size == 2)
  {
    v = *reinterpret_cast<const int16_t *>(data_ptr);
  }
  else if (data_size == 4)
  {
    v = *reinterpret_cast<const int32_t *>(data_ptr);
  }
  else if (data_size == 8)
  {
    v = *reinterpret_cast<const int64_t *>(data_ptr);
  }

  if (factor_in_rip)
  {
    v += elf.ptr_to_va(elf.offset(data));
  }

  return v + offset;
}

DirectExtractor::DirectExtractor(uint64_t offset) : offset(offset)
{
}

uint64_t DirectExtractor::extract(const ElfInterface &elf, const uint8_t *data)
{
  return elf.ptr_to_va(elf.offset(data)) + offset;
}

MenuActionHandlerExtractor::MenuActionHandlerExtractor(csh capstone_handle, uint64_t lea_offset) : capstone_handle(capstone_handle),
                                                                                                   lea_offset(lea_offset)
{
}

uint64_t MenuActionHandlerExtractor::extract(const ElfInterface &elf, const uint8_t *data)
{
  struct Candidate
  {
    uint64_t lea_count;
    uint64_t rip;
  };

  uint64_t lea_count = 0;

  cs_insn *insn;

  auto ret = false;
  if (!data)
  {
    return 0;
  }

  auto rva = elf.ptr_to_va(elf.offset(data));
  std::vector<Candidate> candidates;

  while (!ret)
  {
    cs_disasm(capstone_handle, data, 0x15, static_cast<uint64_t>(rva), 0, &insn);

    if (insn->id == X86_INS_LEA)
    {
      auto x86 = &(insn->detail->x86);
      auto mem = x86->operands[1].mem;

      auto rip = rva + mem.disp + 7;
      if (lea_count == lea_offset)
      {
        return rip;
      }

      candidates.push_back({ lea_count, rip });
      lea_count += 1;
    }
    else if (insn->id == X86_INS_RET)
    {
      ret = true;
    }

    data += insn->size;
    rva += insn->size;
  }

  LOG(WARNING, "Failed to find LEA");
  for (auto &c : candidates)
  {
    LOG(WARNING, " -> Candidate: " << c.lea_count << " " << std::hex << c.rip);
  }
  return 0;
}

CallExtractor::CallExtractor(csh capstone_handle, uint64_t call_offset) : capstone_handle(capstone_handle),
                                                                          call_offset(call_offset)
{
}

uint64_t CallExtractor::extract(const ElfInterface &elf, const uint8_t *data)
{
  uint64_t call_count = 0;

  cs_insn *insn;

  auto ret = false;
  if (!data)
  {
    return 0;
  }

  auto called_addr = data + *reinterpret_cast<const int32_t *>(data + 1) + 5;
  auto rva = elf.ptr_to_va(elf.offset(called_addr));
  while (!ret)
  {
    cs_disasm(capstone_handle, called_addr, 0x15, static_cast<uint64_t>(rva), 0, &insn);

    if (insn->id == X86_INS_CALL)
    {
      if (call_count == call_offset)
      {
        auto x86 = &(insn->detail->x86);
        return static_cast<uint64_t>(x86->operands[0].imm);
      }

      call_count += 1;
    }
    else if (insn->id == X86_INS_RET)
    {
      ret = true;
    }

    called_addr += insn->size;
    rva += insn->size;
  }

  return 0;
}

ConstructorSizeExtractor::ConstructorSizeExtractor(csh capstone_handle, x86_reg reg, uint32_t end) : capstone_handle(capstone_handle),
                                                                                                     reg(reg),
                                                                                                     end(end)
{
}

uint64_t ConstructorSizeExtractor::extract(const ElfInterface &elf, const uint8_t *data)
{
  size_t count;
  cs_insn *insn;

  auto ret = false;
  uint64_t last_written = 0;
  uint8_t last_sz = 0;

  if (!data)
  {
    return 0;
  }

  auto rva = elf.ptr_to_va(elf.offset(data));

  ImportedFunction memset_import;
  elf.find_import("memset", &memset_import);

  while (!ret)
  {
    count = cs_disasm(capstone_handle, data, 0x15, rva, 1, &insn);

    auto x86 = &(insn->detail->x86);
    if (count == 1 && insn->id == X86_INS_MOV)
    {
      if (x86->operands[0].type == x86_op_type::X86_OP_MEM)
      {
        auto op = x86->operands[0];
        auto mem = op.mem;
        if (mem.base == reg)
        {
          if (mem.disp > 0 && static_cast<uint64_t>(mem.disp) > last_written)
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

GenericExtractor::GenericExtractor(void *extractor) : nested(static_cast<Extractor<uint64_t> *>(extractor))
{
}

uint64_t GenericExtractor::extract(const ElfInterface &elf, const uint8_t *data)
{
  return nested->extract(elf, data);
}

DummyExtractor::DummyExtractor(uint64_t v) : v(v)
{
}

uint64_t DummyExtractor::extract(const ElfInterface &, const uint8_t *)
{
  return v;
}

Pattern::Pattern(std::string name, Type type, Extractor<uint64_t> *extractor, bool is_vtf) : name(std::move(name)),
                                                                                             type(std::move(type)),
                                                                                             extractor(extractor),
                                                                                             is_vtf(is_vtf)
{
}

DefaultPattern::DefaultPattern(std::string name, std::vector<int> pattern, Type type, Extractor<uint64_t> *extractor, bool is_vtf) : Pattern(
                                                                                                                                         std::move(name),
                                                                                                                                         std::move(type),
                                                                                                                                         extractor,
                                                                                                                                         is_vtf),
                                                                                                                                     pattern(std::move(pattern))
{
}

const uint8_t *DefaultPattern::find_result(uint8_t *text, Elf64_Shdr text_hdr)
{
  const uint8_t *found;
  auto status = pattern_scan(text, text_hdr.sh_size, pattern, &found);
  if (status == Status::Duplicates)
  {
    LOG(WARNING, "Duplicate pattern");
    return nullptr;
  }
  return found;
}

DummyPattern::DummyPattern(std::string name, Type type, Extractor<uint64_t> *extractor, bool is_vtf) : Pattern(name, type, extractor, is_vtf)
{
}

const uint8_t *DummyPattern::find_result(uint8_t *, Elf64_Shdr)
{
  return nullptr;
}

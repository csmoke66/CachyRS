#pragma once
#include <cstdint>
#include <string>
#include <vector>

#include <elf.h>

#include "type.h"
#include "elf_interface.h"

#include <capstone.h>

template <typename T>
class DataValidator
{
public:
    virtual bool validate(const T *t) = 0;
};

class AlignmentValidator : public DataValidator<uint64_t>
{
private:
    uint64_t alignment;

public:
    AlignmentValidator(uint64_t alignment);

public:
    bool validate(const uint64_t *t) override;
};

//
// Extracts something specific out of a blob of data.
//
template <typename T>
class Extractor
{
private:
    std::vector<DataValidator<T> *> data_validators;

public:
    virtual T extract(const ElfInterface& elf, const uint8_t *data) = 0;

    T extract_validated(const ElfInterface& elf, const uint8_t *data)
    {
        auto t = extract(elf, data);
        for (auto dv : data_validators)
        {
            if (!dv->validate(&t))
            {
                return (T)0;
            }
        }

        return t;
    }

public:
    Extractor<T> *validator(DataValidator<T> *v)
    {
        data_validators.push_back(v);
        return this;
    }
};

//
// Returns an immediate value extracted from a pattern, usually code.
//
// Allows for relative RIP resolving as one of the primary uses of this
// is extracting relative movs/leas that access data within the binary.
//
class ImmExtractor : public Extractor<uint64_t>
{
private:
    uint64_t offset_to_data;
    uint64_t offset;
    size_t data_size;
    bool factor_in_rip;

public:
    ImmExtractor(uint64_t offset_to_data, uint64_t offset, size_t data_size, bool factor_in_rip = false);

public:
    uint64_t extract(const ElfInterface& elf, const uint8_t *data) override;
};

//
// Returns the RVA of the found pattern + offset.
//
class DirectExtractor : public Extractor<uint64_t>
{
    uint64_t offset;

public:
    DirectExtractor(uint64_t offset);

public:
    uint64_t extract(const ElfInterface& elf, const uint8_t *data) override;
};

//
// Exracts menu action handler functions by decoding instructions
// and counting LEAs until the specific LEA loading the function
// is found.
//
class MenuActionHandlerExtractor : public Extractor<uint64_t>
{
private:
    csh capstone_handle;
    uint64_t lea_offset;

public:
    MenuActionHandlerExtractor(csh capstone_handle, uint64_t lea_offset);

public:
    uint64_t extract(const ElfInterface& elf, const uint8_t *data) override;
};

class ConstructorSizeExtractor : public Extractor<uint64_t>
{
private:
    csh capstone_handle;
    x86_reg reg;
    uint32_t end;

public:
    ConstructorSizeExtractor(csh capstone_handle, x86_reg reg, uint32_t end);

public:
    uint64_t extract(const ElfInterface& elf, const uint8_t *data) override;
};

//
// A non-templated extractor that simply passes extraction into
// an inner extractor so there's no need to deal with generics.
//
class GenericExtractor : public Extractor<uint64_t>
{
public:
    Extractor<uint64_t> *nested;

public:
    GenericExtractor(void *extractor);

public:
    uint64_t extract(const ElfInterface& elf, const uint8_t *data) override;
};

//
// An extractor that always returns a constant value. Useful for
// inserting static fields into generated structs.
//
class DummyExtractor : public Extractor<uint64_t>
{
private:
    uint64_t v;

public:
    DummyExtractor(uint64_t v);

public:
    uint64_t extract(const ElfInterface& elf, const uint8_t *data) override;
};

//
// A pattern. This is simply the base type, so it contains no actual pattern data,
// but instead meta-data for the pattern.
//
// This includes the name of the found data, the type of the data, and the extractor
// for extracting the data from the found pattern.
//
class Pattern
{
public:
    std::string name;
    Type type;
    Extractor<uint64_t> *extractor;

public:
    Pattern(std::string name, Type type, Extractor<uint64_t> *extractor);

public:
    virtual const uint8_t *find_result(uint8_t *text, Elf64_Shdr text_hdr) = 0;
};

//
// A default pattern implementation that scans using integers, where -1 refers
// to a wild card.
//
class DefaultPattern : public Pattern
{
public:
    std::vector<int> pattern;

public:
    DefaultPattern(std::string name, std::vector<int> pattern, Type type, Extractor<uint64_t> *extractor);

public:
    virtual const uint8_t *find_result(uint8_t *text, Elf64_Shdr text_hdr);
};

//
// A dummy pattern. Always returns nullptr.
//
class DummyPattern : public Pattern
{
public:
    DummyPattern(std::string name, Type type, Extractor<uint64_t> *extractor);

public:
    virtual const uint8_t *find_result(uint8_t *text, Elf64_Shdr text_hdr);
};

//
// A pattern object. This is a pattern representation of the fields
// of an object (e.g. Engine, Entity, etc.)
//
struct PatternObject
{
    std::string name;
    std::vector<Pattern *> patterns;

    bool is_class = false;
    bool has_parent = false;
    std::string parent;
    
    Pattern* size_pattern;
};

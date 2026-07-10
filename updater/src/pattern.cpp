#include "pattern.h"
#include "updater.h"

ImmExtractor::ImmExtractor(uint64_t offset_to_data, uint64_t offset, size_t data_size, bool factor_in_rip)
{
    this->offset_to_data = offset_to_data;
    this->offset = offset;
    this->data_size = data_size;
    this->factor_in_rip = factor_in_rip;
}

uint64_t ImmExtractor::extract(uint64_t rva, const uint8_t *data)
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
        v += rva;
    }

    return v + offset;
}

GenericExtractor::GenericExtractor(void *extractor)
{
    this->nested = (Extractor<uint64_t> *)extractor;
}

uint64_t GenericExtractor::extract(uint64_t rva, const uint8_t *data)
{
    return nested->extract(rva, data);
}

DummyExtractor::DummyExtractor(uint64_t v)
{
    this->v = v;
}

uint64_t DummyExtractor::extract(uint64_t rva, const uint8_t *data)
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
    const uint8_t* found;
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

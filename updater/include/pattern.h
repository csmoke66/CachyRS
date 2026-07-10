#pragma once
#include <cstdint>
#include <string>
#include <vector>

#include <elf.h>

#include "type.h"

template <typename T>
class Extractor
{
public:
    virtual T extract(uint64_t rva, const uint8_t *data) = 0;
};

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
    virtual uint64_t extract(uint64_t rva, const uint8_t *data);
};

class GenericExtractor : public Extractor<uint64_t>
{
public:
    Extractor<uint64_t> *nested;

public:
    GenericExtractor(void *extractor);

public:
    virtual uint64_t extract(uint64_t rva, const uint8_t *data);
};

class DummyExtractor : public Extractor<uint64_t>
{
private:
    uint64_t v;

public:
    DummyExtractor(uint64_t v);

public:
    virtual uint64_t extract(uint64_t rva, const uint8_t *data);
};

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

class DefaultPattern : public Pattern
{
public:
    std::vector<int> pattern;

public:
    DefaultPattern(std::string name, std::vector<int> pattern, Type type, Extractor<uint64_t> *extractor);

public:
    virtual const uint8_t *find_result(uint8_t *text, Elf64_Shdr text_hdr);
};

class DummyPattern : public Pattern
{
public:
    DummyPattern(std::string name, Type type, Extractor<uint64_t> *extractor);

public:
    virtual const uint8_t *find_result(uint8_t *text, Elf64_Shdr text_hdr);
};

struct PatternObject
{
    std::string name;
    std::vector<Pattern *> patterns;
};


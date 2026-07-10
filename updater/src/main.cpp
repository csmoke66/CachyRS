#include <fstream>
#include <vector>
#include <elf.h>
#include <cstring>
#include <iostream>
#include <sstream>

static std::vector<uint8_t> read_file(const std::string &filePath)
{
    std::ifstream file(filePath, std::ios::binary);

    return std::vector<uint8_t>((std::istreambuf_iterator<char>(file)),
                                std::istreambuf_iterator<char>());
}

static std::vector<int> compile_ida_pattern(const char *pattern)
{
    auto next = pattern;
    std::vector<int> compiled;
    do
    {

        auto tmp = strstr(next, " ");
        if (tmp)
            tmp += 1;

        uint64_t length = 0;
        if (tmp)
        {
            length = (uint64_t)tmp - (uint64_t)next - 1;
        }
        else
        {
            length = strlen(next);
        }

        char number[3] = {0, 0, 0};
        number[0] = next[0];
        if (length > 1)
        {
            number[1] = next[1];
        }

        if (!strcmp(number, "?") || !strcmp(number, "??"))
        {
            compiled.push_back(-1);
        }
        else
        {
            compiled.push_back(strtol(number, NULL, 16));
        }

        next = tmp;
    } while (next);

    return compiled;
}

static const uint8_t *pattern_scan(
    const uint8_t *data,
    size_t size,
    const std::vector<int> &pattern)
{
    for (size_t i = 0; i <= size - pattern.size(); ++i)
    {
        bool found = true;

        for (size_t j = 0; j < pattern.size(); ++j)
        {
            if (pattern[j] != -1 && data[i + j] != pattern[j])
            {
                found = false;
                break;
            }
        }

        if (found)
        {
            return data + i;
        }
    }

    return nullptr;
}

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
    ImmExtractor(uint64_t offset_to_data, uint64_t offset, size_t data_size, bool factor_in_rip = false)
    {
        this->offset_to_data = offset_to_data;
        this->offset = offset;
        this->data_size = data_size;
        this->factor_in_rip = factor_in_rip;
    }

public:
    virtual uint64_t extract(uint64_t rva, const uint8_t *data) override
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
};

class GenericExtractor : public Extractor<uint64_t>
{
public:
    Extractor<uint64_t> *nested;

public:
    GenericExtractor(void *extractor)
    {
        this->nested = (Extractor<uint64_t> *)extractor;
    }

public:
    virtual uint64_t extract(uint64_t rva, const uint8_t *data) override
    {
        return nested->extract(rva, data);
    }
};

class DummyExtractor : public Extractor<uint64_t>
{
private:
    uint64_t v;

public:
    DummyExtractor(uint64_t v)
    {
        this->v = v;
    }

public:
    virtual uint64_t extract(uint64_t rva, const uint8_t *data) override
    {
        return v;
    }
};

struct Type
{
    std::string type;
    size_t size;
    bool arr = false;
    size_t arr_size = 0;
};

class Pattern
{
public:
    std::string name;
    Type type;
    Extractor<uint64_t> *extractor;

public:
    Pattern(std::string name, Type type, Extractor<uint64_t> *extractor)
    {
        this->name = name;
        this->type = type;
        this->extractor = extractor;
    }

public:
    virtual const uint8_t *find_result(uint8_t *text, Elf64_Shdr text_hdr) = 0;
};

class DefaultPattern : public Pattern
{
public:
    std::vector<int> pattern;

public:
    DefaultPattern(std::string name, std::vector<int> pattern, Type type, Extractor<uint64_t> *extractor) : Pattern(name, type, extractor)
    {
        this->pattern = pattern;
    }

public:
    virtual const uint8_t *find_result(uint8_t *text, Elf64_Shdr text_hdr)
    {
        return pattern_scan(text, text_hdr.sh_size, pattern);
    }
};

class DummyPattern : public Pattern
{
public:
    DummyPattern(std::string name, Type type, Extractor<uint64_t> *extractor) : Pattern(name, type, extractor)
    {
    }

public:
    virtual const uint8_t *find_result(uint8_t *text, Elf64_Shdr text_hdr)
    {
        return nullptr;
    }
};

struct PatternObject
{
    std::string name;
    std::vector<Pattern *> patterns;
};

struct Field
{
    std::string name;
    uint64_t offset;
    Type type;
};

struct Object
{
    std::string name;
    std::vector<Field> fields;
};

std::vector<Object> match_to_object(uint8_t *text, Elf64_Shdr text_hdr, std::vector<PatternObject> &patterns)
{
    std::vector<Object> matched;
    for (auto &pattern : patterns)
    {
        Object obj = {pattern.name};
        for (auto pattern : pattern.patterns)
        {
            auto result = pattern->find_result(text, text_hdr);

            auto section_offset = result - text;
            auto file_offset = text_hdr.sh_offset + section_offset;
            auto vaddr = text_hdr.sh_addr + section_offset;

            obj.fields.push_back({pattern->name, pattern->extractor->extract(vaddr, result), pattern->type});
        }
        matched.push_back(obj);
    }
    return matched;
}

std::vector<PatternObject> build_pattern_objects()
{
    // clang-format off

    std::vector<PatternObject> objects;
    objects.push_back({"Globals", {
        new DefaultPattern{
            "engine",
            compile_ida_pattern("4C 89 35 ? ? ? ? 4C 89 FE"),
            { "Engine*", 8, },
            new ImmExtractor(0x3, 0x7, 4, true)},

        new DefaultPattern{
            "menu_execute",
            compile_ida_pattern("E8 ? ? ? ? E9 ? ? ? ? 45 8B AF"),
            {"char", 1},
            new ImmExtractor(0x1, 0x5, 4, true)},
    }});
    
    objects.push_back({"Engine", {
        new DummyPattern{
            "window_state",
            { "WindowState*", 8},
            new DummyExtractor(0x90)},
        new DefaultPattern{
            "world_a",
            compile_ida_pattern("4D 8B B3 ? ? ? ? 41 80 BE"),
            { "WorldA*", 8, },
            new ImmExtractor(0x3, 0x0, 4)},
        new DefaultPattern{
            "scene_001",
            compile_ida_pattern("4C 8B 95 ? ? ? ? 49 63 72"),
            { "Scene001*", 8, },
            new ImmExtractor(0x3, 0x0, 4)},
    }});
    
    objects.push_back({"Scene003", {
        new DefaultPattern{
            "projection_matrix",
            compile_ida_pattern("48 8D 96 ? ? ? ? 48 81 C6 ? ? ? ? 41 8B 88"),
            { "Matrix4x4", 64, },
            new ImmExtractor(0x3, 0x0, 4)},
        new DefaultPattern{
            "world_root",
            compile_ida_pattern("48 8B AF ? ? ? ? 48 89 7C 24"),
            { "WorldNode*", 8, },
            new ImmExtractor(0x3, 0x0, 4)},

            
    }});
    
    objects.push_back({"Entity", {
        new DummyPattern{
            "type",
            { "EntityType", 1},
            new DummyExtractor(0x10)},
        new DefaultPattern{
            "position",
            compile_ida_pattern("F3 44 0F 11 83 ? ? ? ? F3 44 0F 11 8B ? ? ? ? F3 0F 11 83"),
            { "Vec3<float>", 12, },
            new ImmExtractor(0x5, 0x0, 4)},
        new DefaultPattern{
            "name",
            compile_ida_pattern("48 8B 8B ? ? ? ? E9 ? ? ? ? 0F 1F 40 ? 44 8B 83"),
            { "JString", 24, },
            new ImmExtractor(0x3, 0x0, 4)},
    }});

    // clang-format on
    return objects;
}

std::string compile_object_to_structure(Object object)
{
    std::sort(object.fields.begin(), object.fields.end(), [](Field a, Field b)
              { return a.offset < b.offset; });

    std::stringstream ss;
    ss << "struct " << object.name << std::endl;
    ss << "{" << std::endl;
    for (auto i = 0; i < object.fields.size(); i++)
    {
        auto &field = object.fields[i];
        auto is_first = (i == 0);
        if (is_first)
        {
            ss << "\tPAD(0x" << std::hex << field.offset << ");" << std::endl;
        }
        else
        {
            auto &prev = object.fields[i - 1];
            ss << "\tPAD(0x" << std::hex << (field.offset - prev.offset - prev.type.size) << ");" << std::endl;
        }

        ss << "\t" << field.type.type << " " << field.name;
        if (field.type.arr)
        {
            ss << "[" << field.type.arr_size << "]";
        }
        ss << ";";
        ss << std::endl;
    }
    ss << "};" << std::endl;
    for (auto i = 0; i < object.fields.size(); i++)
    {
        auto &field = object.fields[i];
        ss << "static_assert(offsetof(" << object.name << ", " << field.name << ") == 0x" << std::hex << field.offset << ", \"Invalid offset\");" << std::endl;
    }
    ss << std::endl;
    return ss.str();
}

std::string compile_objects_to_header(const std::vector<Object> &objects)
{
    std::stringstream ss;
    ss << "#pragma once" << std::endl;
    ss << "// == Generated by CachyRS updater ==" << std::endl
       << std::endl;

    for (auto &obj : objects)
    {
        ss << "struct " << obj.name << ";" << std::endl;
    }

    ss << std::endl;
    for (auto &obj : objects)
    {
        ss << compile_object_to_structure(obj);
    }
    return ss.str();
}

int main()
{
    auto data = read_file("rs2client");
    auto ehdr = (Elf64_Ehdr *)(data.data());
    auto shdrs = (Elf64_Shdr *)(data.data() + ehdr->e_shoff);
    auto shstrtab = (const char *)(data.data() + shdrs[ehdr->e_shstrndx].sh_offset);

    uint8_t *text;
    Elf64_Shdr text_hdr;

    for (int i = 0; i < ehdr->e_shnum; i++)
    {
        auto &sh = shdrs[i];
        auto name = shstrtab + sh.sh_name;

        if (strcmp(name, ".text") == 0)
        {
            text_hdr = sh;
            text = data.data() + sh.sh_offset;
            break;
        }
    }

    auto pattern_objects = build_pattern_objects();
    auto matched = match_to_object(text, text_hdr, pattern_objects);

    std::ofstream out_header("reversed_generated.h");
    out_header << compile_objects_to_header(matched);
    out_header.close();
}
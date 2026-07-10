#include <fstream>
#include <vector>
#include <elf.h>
#include <cstring>
#include <iostream>

#include "updater.h"
#include "pattern.h"

static std::vector<uint8_t> read_file(const std::string &filePath)
{
    std::ifstream file(filePath, std::ios::binary);

    return std::vector<uint8_t>((std::istreambuf_iterator<char>(file)),
                                std::istreambuf_iterator<char>());
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

        new DefaultPattern{
            "linux_001",
            compile_ida_pattern("48 8B 2D ? ? ? ? 48 83 BD ? ? ? ? ? 74 ? 48 89 FB"),
            { "Linux001*", 8 },
            new ImmExtractor(0x3, 0x7, 4, true)},
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
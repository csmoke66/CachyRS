#include <fstream>
#include <vector>
#include <elf.h>
#include <cstring>
#include <iostream>

#include "updater.h"
#include "pattern.h"

static csh capstone_handle;

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
            "linux_001",
            compile_ida_pattern("48 8B 2D ? ? ? ? 48 83 BD ? ? ? ? ? 74 ? 48 89 FB"),
            { "Linux001*", 8 },
            new ImmExtractor(0x3, 0x7, 4, true)},

        new DefaultPattern{
            "heap",
            compile_ida_pattern("48 89 05 ? ? ? ? 31 C0 ? ? ? 48 89 DE"),
            { "void*", 8 },
            new ImmExtractor(0x3, 0x7, 4, true)},
            
        new DefaultPattern{
            "heap_alloc",
            compile_ida_pattern("E8 ? ? ? ? 44 8B 8C 24 ? ? ? ? 48 89 83"),
            {"char", 1},
            new ImmExtractor(0x1, 0x5, 4, true)},
            
        new DefaultPattern{
            "heap_alloc_aligned",
            compile_ida_pattern("E8 ? ? ? ? 49 89 C7 4D 8B 8D"),
            {"char", 1},
            new ImmExtractor(0x1, 0x5, 4, true)},
            
        new DefaultPattern{
            "engine_tick",
            compile_ida_pattern("41 57 41 56 41 55 41 54 55 53 48 89 FB 48 81 EC ? ? ? ? ? ? ? ? ? E8"),
            {"char", 1},
            new DirectExtractor(0x0)},
            
        new DefaultPattern{
            "menu_execute",
            compile_ida_pattern("E8 ? ? ? ? E9 ? ? ? ? 45 8B AF"),
            {"char", 1},
            new ImmExtractor(0x1, 0x5, 4, true)},

        new DefaultPattern{
            "menu_action_handler_widget",
            compile_ida_pattern("4C 8B 5C 24 ? 4D 89 A5"),
            {"char", 1},
            new MenuActionHandlerExtractor(capstone_handle, 9)},

        new DefaultPattern{
            "menu_action_handler_walk",
            compile_ida_pattern("4C 8B 5C 24 ? 4D 89 A5"),
            {"char", 1},
            new MenuActionHandlerExtractor(capstone_handle, 15)},

        new DefaultPattern{
            "menu_action_handler_npc1",
            compile_ida_pattern("4C 8B 5C 24 ? 4D 89 A5"),
            {"char", 1},
            new MenuActionHandlerExtractor(capstone_handle, 39)},

        new DefaultPattern{
            "menu_action_handler_npc2",
            compile_ida_pattern("4C 8B 5C 24 ? 4D 89 A5"),
            {"char", 1},
            new MenuActionHandlerExtractor(capstone_handle, 41)},

        new DefaultPattern{
            "menu_action_handler_npc3",
            compile_ida_pattern("4C 8B 5C 24 ? 4D 89 A5"),
            {"char", 1},
            new MenuActionHandlerExtractor(capstone_handle, 43)},

        new DefaultPattern{
            "menu_action_handler_npc4",
            compile_ida_pattern("4C 8B 5C 24 ? 4D 89 A5"),
            {"char", 1},
            new MenuActionHandlerExtractor(capstone_handle, 45)},

        new DefaultPattern{
            "menu_action_handler_npc5",
            compile_ida_pattern("4C 8B 5C 24 ? 4D 89 A5"),
            {"char", 1},
            new MenuActionHandlerExtractor(capstone_handle, 47)},

        new DefaultPattern{
            "menu_action_handler_npc6",
            compile_ida_pattern("4C 8B 5C 24 ? 4D 89 A5"),
            {"char", 1},
            new MenuActionHandlerExtractor(capstone_handle, 49)},

        new DefaultPattern{
            "menu_action_handler_npc7",
            compile_ida_pattern("4C 8B 5C 24 ? 4D 89 A5"),
            {"char", 1},
            new MenuActionHandlerExtractor(capstone_handle, 63)},

        new DefaultPattern{
            "menu_action_handler_test_dump",
            compile_ida_pattern("4C 8B 5C 24 ? 4D 89 A5"),
            {"char", 1},
            new MenuActionHandlerExtractor(capstone_handle, 999)},
    }});
    
    objects.push_back({"Engine", {
        
        new DummyPattern{
            "window_state",
            { "WindowState*", 8},
            new DummyExtractor(0x90)},
        new DefaultPattern{
            "time",
            compile_ida_pattern("44 2B A3 ? ? ? ? 41 83 FC ? 0F 87 ? ? ? ? 45 84 C9"),
            { "uint32_t", 4, },
            new ImmExtractor(0x3, 0x0, 4)},
        new DefaultPattern{
            "state",
            compile_ida_pattern("83 BF ? ? ? ? ? 74 ? 6B D6 1E"),
            { "GameState", 4 },
            new ImmExtractor(0x2, 0x0, 4)},
        new DefaultPattern{
            "cache",
            compile_ida_pattern("4C 8B 87 ? ? ? ? 48 8D 8F"),
            { "Cache001*", 8, },
            new ImmExtractor(0x3, 0x0, 4)},
        new DefaultPattern{
            "item_cache",
            compile_ida_pattern("4C 8B AA ? ? ? ? 4D 8B 7D"),
            { "ItemCache*", 8, },
            new ImmExtractor(0x3, 0x0, 4)},
        new DefaultPattern{
            "local_player",
            compile_ida_pattern("48 8B BF ? ? ? ? 48 85 FF 74 ? 48 89 74 24"),
            { "LocalPlayer*", 8, },
            new ImmExtractor(0x3, 0x0, 4)},
        new DefaultPattern{
            "entity_update_cache",
            compile_ida_pattern("49 8B B4 24 ? ? ? ? 0F 84 ? ? ? ? 44 8B 4A"),
            { "EntityUpdateCache*", 8, },
            new ImmExtractor(0x4, 0x0, 4)},
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
        new DefaultPattern{
            "status",
            compile_ida_pattern("4D 8B 95 ? ? ? ? 89 C1"),
            { "EntityStatus*", 4, },
            new ImmExtractor(0x3, 0x0, 4)},

            
    }});


    // clang-format on
    return objects;
}

int main()
{
    cs_open(CS_ARCH_X86, CS_MODE_64, &capstone_handle);
    cs_option(capstone_handle, CS_OPT_DETAIL, CS_OPT_ON);

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
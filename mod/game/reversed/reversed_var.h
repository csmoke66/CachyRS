#pragma once

#pragma pack(push, 1)
struct Stat
{
    // 0x0
    PAD(0xc);
    // 0xc
    uint32_t experience;
    // 0x10
    uint32_t current_level;
    // 0x14
    uint32_t max_level;
    // 0x18
};

struct LocalPlayerVariables
{
    // 0x0
    PAD(0x8);
    // 0x8
    uint32_t stat_count;
    // 0xc
    PAD(0x4);
    // 0x10
    Stat* stats;
    // 0x18
    uint32_t run_energy;
    // 0x1c
};

struct VariableCache
{
    // 0x0
    PAD(0x73a0);
    /// 0x73a0
    LocalPlayerVariables* local_player_variables;
};

#pragma pack(pop)
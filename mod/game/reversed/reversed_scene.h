#pragma once

#pragma pack(push, 1)
struct TerrainHeightData
{

};

struct TerrainGrid002
{
    PAD(0x160);
    // 0x160
    JVector<TaggedObject<void, JVector<TerrainHeightData>>> height_data_1; 
    // 0x178
    PAD(0x70);
    JVector<TerrainHeightData>* height_data_2;
};

struct TerrainGrid001
{
    // 0x0
    PAD(0x8);
    // 0x8
    TerrainGrid002* grid_002;
    // 0x10
    void *unknown;
};

struct Terrain
{
    // 0x0
    PAD(0x14020);
    // 0x14020
    JVector<JVector<TerrainGrid001>> grid;
};

struct WorldNode
{
    // 0x0
    PAD(0x30);
    // 0x30
    Vec3<float> pos_b;
    // 0x3c
    PAD(0x4);
    // 0x40
    Vec3<float> pos_a;
    // 0x4c
    PAD(0x4);
    // 0x50
    Vec3<float> pos_c;
    // 0x5c
    PAD(0x24);
    // 0x80
    Vec3<float> pos_avg;
    // 0x8c
    PAD(0x70);
    // 0xfc
    WorldNodeFlag flags;
    // 0x100
    PAD(0x3a);
    // 0x138
    JVector<WorldNode *> children;
    // 0x150
    PAD(0x50);
    // 0x1a0
    Entity *entity;
    // 0x1d8
};
static_assert(off(WorldNode, children) == 0x138, INVALID_OFFSET);
static_assert(off(WorldNode, entity) == 0x1a0, INVALID_OFFSET);

struct Scene002
{
    // 0x0
    PAD(0x8);
    // 0x8
    Scene003 *scene_003;
};

struct Scene001
{
    // 0x0
    PAD(0x58);
    // 0x58
    JVector<Scene002> scene_002;
    // 0x70
    int32_t scene_index;
};
static_assert(off(Scene001, scene_index) == 0x70, INVALID_OFFSET);
#pragma pack(pop)
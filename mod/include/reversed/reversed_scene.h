#pragma once
#include "reversed_util.h"
#include "reversed_fwd_decl.h"

#pragma pack(push, 1)
struct WorldNode
{
	// 0x0
    PAD(0x50);
	// 0x50
	const Vec3<const float> max;
	// 0x5c
    PAD(0x24);
	// 0x80
	const Vec3<const float> min;
	// 0x8c
    PAD(0xAC);
	// 0x138
	const JArray<const WorldNode*> children;
	// 0x148
    PAD(0x58);
	// 0x1a0
	const Entity* entity;
	// 0x1d8
};
static_assert(off(WorldNode, children) == 0x138, INVALID_OFFSET);
static_assert(off(WorldNode, entity) == 0x1a0, INVALID_OFFSET);

struct Scene002
{
    // 0x0
    PAD(0x8);
    // 0x8
    const Scene003* scene_003;
};

struct Scene001
{
    // 0x0
    PAD(0x58);
    // 0x58
    const JArray<const Scene002> scene_002;
    // 0x68
    PAD(0x8);
    // 0x70
    const int32_t scene_index;
};
static_assert(off(Scene001, scene_index) == 0x70, INVALID_OFFSET);
#pragma pack(pop)
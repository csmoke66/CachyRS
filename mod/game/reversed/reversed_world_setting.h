#pragma once

#pragma pack(push, 1)
struct WorldSettingMask
{
	// 0x0
	PAD(0x48);
	// 0x48
	uint32_t begin;
	// 0x4c
	uint32_t end;
};

struct WorldSetting
{
	// 0x0
	const uint32_t id;
	// 0x4
	PAD(0x4);
	// 0x8
	const uint32_t value;
	// 0xc
	PAD(0x1c);
	// 0x28
	const WorldSetting* next;
};
static_assert(off(WorldSetting, next) == 0x28, INVALID_OFFSET);

struct WorldSettingCache
{
	// 0x0
	PAD(0x28);
	// 0x28
	const WorldSetting** vars;
	// 0x30
	const uint32_t count;
};
static_assert(sizeof(WorldSettingCache) == 0x34, INVALID_SIZE);
#pragma pack(pop)
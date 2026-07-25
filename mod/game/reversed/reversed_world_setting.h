#pragma once

#pragma pack(push, 1)
struct WorldSettingMask
{
	// 0x0
	PAD(0x38);
	// 0x38
	uint32_t world_setting_id;
	// 0x3c
	PAD(0xc);
	// 0x48
	uint32_t begin;
	// 0x4c
	uint32_t end;
};
static_assert(off(WorldSettingMask, begin) == 0x48, INVALID_OFFSET);

struct WorldSetting
{
	// 0x0
	const uint32_t value;
	// 0x4
	PAD(0x14);
	// 0x18
	uint8_t uninitialized;
	// 0x19
	PAD(0x7);
	// 0x20
	const IdObject<uint32_t, 0x4, WorldSetting> *next;
};
static_assert(off(WorldSetting, next) == 0x20, INVALID_OFFSET);

// This class actually extends MUCH further, and uses a list much
// further down internally. I'm not sure why.
class WorldSettingCache
{
public:
	// 0x0
	PAD_VT();
	// 0x8

public:
	// 0x8
	PAD(0x20);
	// 0x28
	const IdObject<uint32_t, 0x4, WorldSetting> **vars;
	// 0x30
	const uint32_t count;
};
static_assert(sizeof(WorldSettingCache) == 0x34, INVALID_SIZE);
#pragma pack(pop)
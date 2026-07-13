#pragma once
#include "reversed_util.h"

struct WorldSetting
{
	// 0x0
	uint32_t id;
	// 0x4
	PAD(0x4);
	// 0x8
	uint32_t value;
	// 0xc
	PAD(0x1c);
	// 0x28
	WorldSetting* next;
};
static_assert(off(WorldSetting, next) == 0x28, INVALID_OFFSET);

struct WorldSettingCache
{
	// 0x0
	uint32_t count;
	// 0x4
	PAD(0x8);
	// 0xc
	WorldSetting* vars[1];
};

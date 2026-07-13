#pragma once
#include "reversed_util.h"

#pragma pack(push, 1)
struct ItemContainer
{
	PAD(0x10);
	// 0x10
	const uint32_t id;
	// 0x14
	PAD(0x4);
	// 0x18
	const JArray<const Item> items;
	// 0x28
	PAD(0x20);
	// 0x48
};
static_assert(sizeof(ItemContainer) == 0x48, INVALID_SIZE);

struct ItemCache
{
	// 0x0
	PAD(0x8);
	// 0x8
	const JArray<const ItemContainer> containers;
	// 0x18
};
#pragma pack(pop)

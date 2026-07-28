#pragma once

#pragma pack(push, 1)
struct ItemContainer
{
	PAD(0x10);
	// 0x10
	const uint32_t id;
	// 0x14
	PAD(0x4);
	// 0x18
	const JVector<const Item> items;
	// 0x30
	PAD(0x18);
	// 0x48
};
static_assert(sizeof(ItemContainer) == 0x48, INVALID_SIZE);

struct ItemCache
{
	// 0x0
	PAD(0x8);
	// 0x8
	const JVector<const ItemContainer> containers;
	// 0x20
};
#pragma pack(pop)

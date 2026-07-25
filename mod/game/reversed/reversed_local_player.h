#pragma once

#pragma pack(push, 1)
struct LocalPlayer
{
	// 0x0
	PAD(0x48);
	// 0x48
	const int32_t entity_list_index;
	// 0x4c
	PAD(0x48);
	// 0x68
	const char name[64];
};
#pragma pack(pop)
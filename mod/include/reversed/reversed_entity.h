#pragma once

#pragma pack(push, 1)
struct StatusBarConfig
{
	// 0x0
	PAD(0x8);
	// 0x8
	const uint32_t id;
};

struct StatusBarData
{
	// 0x0
	PAD(0x10);
	// 0x10
	const StatusBarConfig* config;
	// 0x18
	PAD(0x18);
	// 0x30
	const int32_t display_time;
	// 0x34
	const uint8_t value;
};
static_assert(off(StatusBarData, display_time) == 0x30, INVALID_SIZE);

struct StatusBar
{
	// 0x0
	const StatusBarData* data;
	// 0x8
	PAD(0x1a8);
	// 0x1b0
};
static_assert(sizeof(StatusBar) == 0x1b0, INVALID_SIZE);

struct EntityStatus
{
	// 0x0
	PAD(0x28);
	// 0x28
	const JArray<const StatusBar> bars;
	// 0x30
};

#pragma pack(pop)
#pragma once

#pragma pack(push, 1)
struct StatusBarConfig
{
	// 0x0
	PAD(0x8);
	// 0x8
	uint32_t id;
};

struct StatusBarData
{
	// 0x0
	PAD(0x10);
	// 0x10
	StatusBarConfig* config;
	// 0x18
	PAD(0x18);
	// 0x30
	int32_t display_time;
	// 0x34
	uint8_t value;
};
static_assert(off(StatusBarData, display_time) == 0x30, INVALID_OFFSET);

struct StatusBar
{
	// 0x0
	StatusBarData* data;
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
	JVector<StatusBar> bars;
	// 0x38
};

struct MovementPoint
{
	PAD(0x4);
	// 0x4
	float scene_x;
	// 0x8
	PAD(0x4);
	// 0xc
	float scene_y;
	// 0x10
	PAD(0x8);
	// 0x18
};

struct MovementQueue
{
	// 0x0
	PAD(0x28);
	// 0x28
	JArray2<MovementPoint> points;
	// 0x38
};
#pragma pack(pop)
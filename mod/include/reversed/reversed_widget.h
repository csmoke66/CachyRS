#pragma once

#pragma pack(push, 1)
class Widget
{
public:
	// 0x0
	PAD_VT();
	// 0x8
	virtual uint8_t get_type() const = 0;
	// 0x10
	PAD_VT();
	// 0x18
	PAD_VT();
	// 0x20
	PAD_VT();
	// 0x28
	PAD_VT();
	// 0x30
	PAD_VT();
	// 0x38
	PAD_VT();
	// 0x40
	PAD_VT();
	// 0x48
	PAD_VT();
	// 0x50
	PAD_VT();
	// 0x58
	virtual int32_t get_item_stack_size() const = 0;
	// 0x60

public:
	// 0x8
	const uint16_t parent_id;
	// 0xa
	const uint16_t child_id;
	// 0x16
	PAD(0xc);
	// 0x18
	const ContainerWidget* parent;
	// 0x20
	PAD(0x30);
	// 0x50
	const uint32_t x;
	// 0x54
	const uint32_t y;
	// 0x58
	const uint32_t width;
	// 0x5c
	const uint32_t height;
private:
	// 0x60
	PAD(0xa8);
	// 0x108
};
static_assert(off(Widget, x) == 0x50, INVALID_OFFSET);
static_assert(off(Widget, y) == 0x54, INVALID_OFFSET);
static_assert(sizeof(Widget) == 0x108, INVALID_SIZE);

struct WidgetChild
{
	// 0x0
	const uint32_t id;
	// 0x4
	const uint32_t unknown;
	// 0x8
	PAD(0x8);
	// 0x10
	const Widget* internal;
	// 0x18
};

class ContainerWidget : public Widget
{
public:
	// 0x108
	PAD(0x70);
	// 0x178
	const JArray<WidgetChild> children;
	// 0x188
	PAD(0x20);
	// 0x1a8
	const JArray<WidgetChild> item_children;
};
static_assert(off(ContainerWidget, children) == 0x178, INVALID_OFFSET);
static_assert(off(ContainerWidget, item_children) == 0x1a8, INVALID_OFFSET);

class MediaWidget : public Widget
{
public:
	// 0x108
	PAD(0x48);
	// 0x150
	const int32_t texture_id_disabled;
	// 0x154
	PAD(0x14);
	// 0x168
	const int32_t texture_id_enabled;
	// 0x16c
	PAD(0x14);
	// 0x180
	const uint32_t item_id;
	// 0x184
	PAD(0x4);
	// 0x188
	const uint32_t item_amount;
	// 0x18c
};
static_assert(off(MediaWidget, item_id) == 0x180, INVALID_OFFSET);

class TextWidget : public Widget
{
public:
	// 0x108
	PAD(0x58);
	// 0x160
	const JString text;
	// 0x178
};

struct MiniMapWidgetData
{
	// 0x0
	PAD(0x480);
	// 0x480
	const float scale;
	// 0x484
	PAD(0x94);
	// 0x518
	const float rotation;
};
static_assert(off(MiniMapWidgetData, scale) == 0x480, INVALID_OFFSET);
static_assert(off(MiniMapWidgetData, rotation) == 0x518, INVALID_OFFSET);

class MiniMapWidget : public Widget
{
public:
	// 0x108
	PAD(0xe8);
	// 0x1f0
	const MiniMapWidgetData* minimap_data;
	// 0x1f8
};
static_assert(off(MiniMapWidget, minimap_data) == 0x1f0, INVALID_OFFSET);

struct Widget003
{
	// 0x0
	PAD(0x10);
	// 0x10
	const Widget* internal;
	// 0x18
};

struct Widget002
{
	// 0x0
	const uint32_t id;
	// 0x4
	PAD(0x1c);
	// 0x20
	const JArray<Widget003> widgets_003;
	// 0x30
};

struct Widget001
{
	// 0x0
	PAD(0x8);
	// 0x8
	const Widget002* widget_002;
};

struct WidgetCache
{
	// 0x0
	PAD(0x88);
	// 0x88
	const JArray<Widget001> c;
};
#pragma pack(pop)
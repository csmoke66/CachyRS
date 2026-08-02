#pragma once

#pragma pack(push, 1)
class Widget
{
public:
	// 0x0
	PAD_VT();
	// 0x8
	PAD_VT();
	// 0x10
	virtual WidgetType get_type() const = 0;

public:
	// 0x8
	uint16_t parent_id;
	// 0xa
	uint16_t child_id;
	// 0x16
	PAD(0xc);
	// 0x18
	ContainerWidget* parent;
	// 0x20
	PAD(0x30);
	// 0x50
	uint32_t x;
	// 0x54
	uint32_t y;
	// 0x58
	uint32_t width;
	// 0x5c
	uint32_t height;
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
	uint32_t id;
	// 0x4
	uint32_t unknown;
	// 0x8
	PAD(0x8);
	// 0x10
	Widget* widget;
	// 0x18
};

class ContainerWidget : public Widget
{
public:
	// 0x108
	PAD(0x70);
	// 0x178
	JVector<WidgetChild> children;
	// 0x190
	PAD(0x18);
	// 0x1a8
	JVector<WidgetChild> item_children;
};
static_assert(off(ContainerWidget, children) == 0x178, INVALID_OFFSET);
static_assert(off(ContainerWidget, item_children) == 0x1a8, INVALID_OFFSET);

class MediaWidget : public Widget
{
public:
	// 0x108
	PAD(0x48);
	// 0x150
	int32_t texture_id_disabled;
	// 0x154
	PAD(0x14);
	// 0x168
	int32_t texture_id_enabled;
	// 0x16c
	PAD(0x14);
	// 0x180
	uint32_t item_id;
	// 0x184
	PAD(0x4);
	// 0x188
	uint32_t item_amount;
	// 0x18c
};
static_assert(off(MediaWidget, item_id) == 0x180, INVALID_OFFSET);

class TextWidget : public Widget
{
public:
	// 0x108
	PAD(0x58);
	// 0x160
	JString text;
	// 0x178
};

struct MiniMapWidgetData
{
	// 0x0
	PAD(0x480);
	// 0x480
	float scale;
	// 0x484
	PAD(0x94);
	// 0x518
	float rotation;
};
static_assert(off(MiniMapWidgetData, scale) == 0x480, INVALID_OFFSET);
static_assert(off(MiniMapWidgetData, rotation) == 0x518, INVALID_OFFSET);

class MiniMapWidget : public Widget
{
public:
	// 0x108
	PAD(0xe8);
	// 0x1f0
	MiniMapWidgetData* minimap_data;
	// 0x1f8
};
static_assert(off(MiniMapWidget, minimap_data) == 0x1f0, INVALID_OFFSET);

struct Widget003
{
	// 0x0
	PAD(0x10);
	// 0x10
	Widget* widget;
	// 0x18
};

struct Widget002
{
	// 0x0
	uint32_t id;
	// 0x4
	PAD(0x1c);
	// 0x20
	JVector<Widget003> widgets_003;
	// 0x38
};

struct Widget001
{
	// 0x0
	PAD(0x8);
	// 0x8
	Widget002* widget_002;
};

struct WidgetCache
{
	// 0x0
	PAD(0x68);
	// 0x68
	JVector<Widget001> c;
	// 0x80
	PAD(0x1a8);
	// 0x228
	uint8_t widget_item_selected;
	// 0x229
};
static_assert(off(WidgetCache, widget_item_selected) == 0x228, INVALID_OFFSET);

#pragma pack(pop)
#pragma once
#include <EGL/egl.h>
#include <SDL2/SDL.h>
#include <cstdint>

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wuninitialized"
#pragma clang diagnostic ignored "-Winvalid-offsetof"

#include "reversed_util.h"
#include "reversed_manual_base.h"

struct Linux005;
struct Linux004;
struct Linux003;
struct Linux002;
struct Linux001;
struct WindowState;
struct WorldNode;
struct Scene002;
struct Scene001;
struct WorldA;
class MenuActionTemplate;
struct MenuActionContext;
struct ActionMenuContext;
class Cache003;
struct Cache002;
struct CacheBuffer;
class CacheIndexInner;
struct CacheIndex;
struct Cache001;
struct LocalPlayer;
struct EntityUpdate;
struct EntityUpdateCache;
struct Item;
struct ItemContainer;
struct ItemCache;
struct EntityStatus;
struct WorldSettingCache;
struct WidgetCache;
class ContainerWidget;

enum class EntityType : uint8_t
{
	npc = 1,
	player = 2,
	ground_item = 3,
	effect = 4,
	object1 = 0,
	object2 = 12
};

enum class GameState : uint32_t
{
	login_screen = 10,
	lobby_screen = 20,
	in_game = 30
};

#include "reversed_generated.h"

#pragma pack(push, 1)
struct Linux005
{
	// 0x0
	PAD(0x28);
	// 0x28
	const SDL_Window* sdl_window;
};

struct Linux004
{
	// 0x0
	PAD(0x90);
	// 0x90
	const Linux005* linux_005;
};

struct Linux003
{
	// 0x0
	PAD(0x8);
	// 0x8
	const Linux004* linux_004;
};

struct Linux002
{
	// 0x0
	PAD(0x30);
	// 0x30
	const Linux003* linux_003;
};

struct Linux001
{
	// 0x0
	PAD(0x1b8);
	// 0x1b8
	const Linux002* linux_002;
};

struct WindowState
{
	// 0x0
	PAD(0x28);
	// 0x28
	const SDL_Window* window;
};

struct WorldNode
{
	// 0x0
    PAD(0x50);
	// 0x50
	const Vec3<const float> max;
	// 0x5c
    PAD(0x24);
	// 0x80
	const Vec3<const float> min;
	// 0x8c
    PAD(0xAC);
	// 0x138
	const JArray<const WorldNode*> children;
	// 0x148
    PAD(0x58);
	// 0x1a0
	const Entity* entity;
	// 0x1d8
};
static_assert(off(WorldNode, children) == 0x138, INVALID_OFFSET);
static_assert(off(WorldNode, entity) == 0x1a0, INVALID_OFFSET);

struct Scene002
{
    // 0x0
    PAD(0x8);
    // 0x8
    const Scene003* scene_003;
};

struct Scene001
{
    // 0x0
    PAD(0x58);
    // 0x58
    const JArray<const Scene002> scene_002;
    // 0x68
    PAD(0x8);
    // 0x70
    const int32_t scene_index;
};
static_assert(off(Scene001, scene_index) == 0x70, INVALID_OFFSET);

struct WorldA
{

};

class MenuActionTemplate
{
public:
	// 0x0
	PAD(0x18);
	// 0x18
	const void* handler;
};

struct MenuActionContext
{
	// 0x0
	PAD(0x38);
	// 0x38
	const MenuActionTemplate* tmpl;
	// 0x40
	PAD(0x8);
	// 0x48
	union
	{
		const int32_t args[4];
		struct 
		{
			const uint32_t option_idx;
			const uint32_t sub_idx;
			const uint32_t widget_id;
			const uint32_t always_1;
		} args_widget;
		struct
		{
			const uint32_t server_idx;
			const uint32_t always_0_0;
			const uint32_t always_0_1;
			const uint32_t always_1;
		} args_npc;
	};
};

struct ActionMenuContext
{
	// 0x0
	PAD(0x8);
	// 0x8
	const MenuActionContext* menu_action_context;
	// 0x10
};


class Cache003
{
public:
	// 0x0
	PAD_VT();
	// 0x8
	PAD_VT();
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
	virtual void* test(int id, bool unk) = 0;
};

struct Cache002
{
	// 0x0
	PAD(0x40);
	// 0x40
	const Cache003* cache_003;
	// 0x48
};

struct CacheBuffer
{
	// 0x0
	const void* tag;
	// 0x8
	const void* body;
	// 0x10
};

class CacheIndexInner
{
private:
	// 0x0
	PAD_VT();
	// 0x8
	PAD_VT();
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

public:
	// 0x38
	virtual void* get_cache_descriptor_by_index(uint32_t idx, bool unknown) = 0;

public:
	// 0x8
	PAD(0x38);
	// 0x40
	const uint64_t size;
	// 0x48
	const JArray<const CacheBuffer**> data_buffer;
};

struct CacheIndex
{
	// 0x0
	PAD(0x30);
	// 0x30
	const uint32_t current_size;
	// 0x34
	const uint32_t max_size;
	// 0x38
	PAD(0x8);
	// 0x40
	const CacheIndexInner* inner;
	// 0x48
};

struct Cache001
{
	// 0x0
	PAD(0xa8);
	// 0xa8
	const CacheIndex** indices;
	// 0xb0
	PAD(0x110);
	// 0x1c0
	const Cache002* cache_002;
	// 0x1c8
};

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

struct EntityUpdate
{
	// 0x0
	PAD(0x38);
	// 0x38
	const Entity* entity;
	// 0x40
};

struct EntityUpdateCache
{
	// 0x0
	PAD(0x10);
	// 0x10
	const JArray<const EntityUpdate*> updates;
	// 0x20
};
struct Item
{
	// 0x0
	const int32_t id;
	// 0x4
	const int32_t amount;
	// 0xc
};

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

#pragma pack(pop)

typedef EGLBoolean (*FnEglSwapBuffers)(EGLDisplay dpy, EGLSurface surface);
typedef int (*FnSDL_PollEvent)(SDL_Event* event);
typedef void* (*FnMenuExecute)(void* menu_context, const ActionMenuContext* context, void* unk_003);
typedef void* (*FnHeapAllocate)(void* heap, size_t size, size_t alignment);
typedef void* (*FnHeapAllocateAligned)(size_t size);

#pragma GCC diagnostic pop
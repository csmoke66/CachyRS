#pragma once

#pragma pack(push, 1)
class MenuActionTemplate
{
public:
	// 0x0
	PAD(0x18);
	// 0x18
	void *handler;
	// 0x20
	uint32_t id;
	// 0x24
	uint32_t type;
	// 0x28
};

enum class MenuActionType : uint8_t
{
	unknown,
};

union MenuActionArgs
{
	int32_t r[4];
	struct
	{
		uint32_t option_idx;
		uint32_t sub_idx;
		uint32_t widget_id;
		uint32_t always_1;
	} args_widget;
	struct
	{
		uint32_t server_idx;
		uint32_t always_0_0;
		uint32_t always_0_1;
		uint32_t always_1;
	} args_npc;
};

struct MenuActionContext
{
	// 0x0
	PAD(0x38);
	// 0x38
	MenuActionTemplate *tmpl;
	// 0x40
	PAD(0x8);
	// 0x48
	MenuActionArgs args;
};

struct ActionMenuContext
{
	// 0x0
	PAD(0x8);
	// 0x8
	MenuActionContext *menu_action_context;
	// 0x10
};

struct MenuOption002
{
	// 0x0
	PAD(0x58);
	// 0x58
	MenuActionTemplate* action_template;
	// 0x60
	PAD(0xf0);
	// 0x150
};
static_assert(sizeof(MenuOption002) == 0x150, INVALID_SIZE);

struct MenuOption001
{
	PAD(0x70);
	// 0x70
	MenuOption002* menu_option_002;
};

struct Menu
{
	PAD(0x13a0);
	// 0x13a0
	JArray<MenuOption001*> menu_options;
	// 0x13b8
};
#pragma pack(pop)
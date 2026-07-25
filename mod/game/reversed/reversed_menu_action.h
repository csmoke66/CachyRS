#pragma once

#pragma pack(push, 1)
class MenuActionTemplate
{
public:
	// 0x0
	PAD(0x18);
	// 0x18
	void *handler;
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
#pragma pack(pop)
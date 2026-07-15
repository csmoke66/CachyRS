#pragma once

#pragma pack(push, 1)
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
#pragma pack(pop)
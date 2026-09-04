#pragma once

#pragma pack(push, 1)
typedef void *(*FnMenuActionHandler)(MenuActionTemplate *, ActionMenuContext *ctx);

class MenuActionTemplate
{
public:
  // 0x0
  Engine *engine;
  // 0x8
  PAD(0x10);
  // 0x18
  FnMenuActionHandler handler;
  // 0x20
  uint32_t id;
  // 0x24
  MenuActionType type;
  // 0x28
};

union MenuActionArgs
{
  // 0x48
  // 0x4c
  // 0x50
  // 0x54
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
  struct
  {
    uint32_t object_id;
    uint32_t tile_x;
    uint32_t tile_y;
    uint32_t always_1;
  } args_obj;
  struct
  {
    uint32_t always_0_maybe;
    uint32_t slot;
    uint32_t widget_id;
    uint32_t always_1_maybe;
  } args_select_item;
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

struct MenuOption
{
  // 0x0
  PAD(0x18);
  // 0x18
  JString option_text;
  // 0x30
  PAD(0x8);
  // 0x38
  MenuActionTemplate *action_template;
  // 0x40
  PAD(0x28);
  // 0x68
  JString entity_text;
  // 0x80
  PAD(0xb0);
  // 0x130
};
static_assert(sizeof(MenuOption) == 0x130, INVALID_SIZE);

struct Menu
{
  // 0x0
  PAD(0x68);
  // 0x68
  uint8_t is_open;
  // 0x69
  PAD(0xcc7);
  // 0xd30
  JVector<TaggedObject<void, MenuOption>> menu_options;
};
static_assert(off(Menu, menu_options) == 0xd30, INVALID_SIZE);
#pragma pack(pop)
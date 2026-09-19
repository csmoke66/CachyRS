#pragma once
#include "reversed_enum.h"
#include "reversed_fwd_decl.h"
#include "reversed_manual_base.h"
#include "reversed_util.h"

class Widget;

#pragma pack(push, 1)
struct WidgetMenuOption
{
  // 0x0
  JString text;
  // 0x18
  PAD(0x28);
};
static_assert(sizeof(WidgetMenuOption) == 0x40, INVALID_SIZE);

struct WidgetChild
{
  // 0x0
  PAD(0x10);
  // 0x10
  Widget *widget;
  // 0x18
};
struct Widget003
{
  // 0x0
  PAD(0x10);
  // 0x10
  Widget *widget;
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
  Widget002 *widget_002;
};

struct WidgetCache
{
  // 0x0
  PAD(0x50);
  // 0x50
  JVector<Widget001> c;
  // 0x68
  PAD(0x1b0);
  // 0x218
  uint8_t widget_item_selected;
  // 0x229
};
static_assert(off(WidgetCache, widget_item_selected) == 0x218, INVALID_OFFSET);

#pragma pack(pop)
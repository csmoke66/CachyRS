#include "game_events.h"

#include <algorithm>
#include <cstring>

namespace crs
{
  static void copy_cstr(char *dst, size_t dst_size, const std::string &src)
  {
    if (dst_size == 0)
    {
      return;
    }

    const auto n = std::min(src.size(), dst_size - 1);
    std::memcpy(dst, src.data(), n);
    dst[n] = '\0';
  }
  EngineTickEvent::EngineTickEvent(Engine *engine) : Event(EngineTickEvent::specific_id()),
                                                     args{ .engine = engine }
  {
  }

  void *EngineTickEvent::get_args()
  {
    return &args;
  }

  MenuOpenedEvent::MenuOpenedEvent(bool opened) : Event(MenuOpenedEvent::specific_id()),
                                                  args{ .opened = opened }
  {
  }

  void *MenuOpenedEvent::get_args()
  {
    return &args;
  }

  MenuActionEvent::MenuActionEvent(std::string_view id, MenuActionArgs *args, MenuActionTemplate **action_template) : Event(id),
                                                                                                                      args{ .args = args, .action_template = action_template }
  {
  }

  void *MenuActionEvent::get_args()
  {
    return &args;
  }

  WorldSettingChangedEvent::WorldSettingChangedEvent(uint32_t world_setting_id, uint32_t value) : Event(WorldSettingChangedEvent::specific_id()),
                                                                                                  args{ .world_setting_id = world_setting_id, .value = value }
  {
  }

  void *WorldSettingChangedEvent::get_args()
  {
    return &args;
  }

  ItemChangedEvent::ItemChangedEvent(uint32_t id, uint32_t slot, int32_t old_id, int32_t old_amount, int32_t new_id, int32_t new_amount, int32_t stack_delta) : Event(ItemChangedEvent::specific_id()),
                                                                                                                                                                args{ .id = id, .slot = slot, .old_id = old_id, .old_amount = old_amount, .new_id = new_id, .new_amount = new_amount, .stack_delta = stack_delta }
  {
  }

  void *ItemChangedEvent::get_args()
  {
    return &args;
  }

  NewChatMessageEvent::NewChatMessageEvent(const std::string &channel, const std::string &sender, const std::string &message) : Event(NewChatMessageEvent::specific_id())
  {
    copy_cstr(args.channel, sizeof(args.channel), channel);
    copy_cstr(args.sender, sizeof(args.sender), sender);
    copy_cstr(args.message, sizeof(args.message), message);
  }

  void *NewChatMessageEvent::get_args()
  {
    return &args;
  }
} // namespace crs
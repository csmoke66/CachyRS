#pragma once
#include "event_bus.h"

namespace crs
{
  struct EngineTickArgs
  {
    Engine *engine;
  };

  class EngineTickEvent : public Event
  {
  public:
    EngineTickArgs args;

  public:
    static constexpr std::string specific_id()
    {
      return "on_engine_tick";
    }

  public:
    EngineTickEvent(Engine *engine);

  public:
    void *get_args() override;
  };

  struct MenuOpenedEventArgs
  {
    bool opened;
  };

  class MenuOpenedEvent : public Event
  {
  public:
    MenuOpenedEventArgs args;

  public:
    static constexpr std::string specific_id()
    {
      return "on_menu_opened";
    }

  public:
    MenuOpenedEvent(bool opened);

  public:
    void *get_args() override;
  };

  struct MenuActionEventArgs
  {
    MenuActionArgs *args;
    MenuActionTemplate **action_template;
    bool bypass_logic = false;
  };

  class MenuActionEvent : public Event
  {
  public:
    MenuActionEventArgs args;

  public:
    static constexpr std::string pre_id()
    {
      return "on_menu_action_pre";
    }

    static constexpr std::string post_id()
    {
      return "on_menu_action_post";
    }

  public:
    MenuActionEvent(const std::string &id, MenuActionArgs *args, MenuActionTemplate **action_template);

  public:
    void *get_args() override;
  };

  struct WorldSettingChangedEventArgs
  {
    uint32_t world_setting_id;
    uint32_t value;
  };

  class WorldSettingChangedEvent : public Event
  {
  public:
    WorldSettingChangedEventArgs args;

  public:
    static constexpr std::string specific_id()
    {
      return "on_set_varbit";
    }

  public:
    WorldSettingChangedEvent(uint32_t world_setting_id, uint32_t value);

  public:
    void *get_args() override;
  };

  struct ItemChangedArgs
  {
    uint32_t id;
    uint32_t slot;
    int32_t old_id;
    int32_t old_amount;
    int32_t new_id;
    int32_t new_amount;
    int32_t stack_delta;
  };

  class ItemChangedEvent : public Event
  {
  public:
    ItemChangedArgs args;

  public:
    static constexpr std::string specific_id()
    {
      return "on_set_item_container";
    }

  public:
    ItemChangedEvent(uint32_t id, uint32_t slot, int32_t old_id, int32_t old_amount, int32_t new_id, int32_t new_amount, int32_t stack_delta);

  public:
    void *get_args() override;
  };

} // namespace crs
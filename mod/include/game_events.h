#pragma once
#include "event_bus.h"

namespace crs
{
    class EngineTickEvent : public Event
    {
    public:
        static constexpr ::std::string specific_id()
        {
            return "on_engine_tick";
        }

    public:
        EngineTickEvent();
    };

    struct MenuActionEventArgs
    {
        MenuActionType type;
        MenuActionArgs *args;
    };

    class MenuActionEvent : public Event
    {
    public:
        MenuActionEventArgs args;

    public:
        static constexpr ::std::string specific_id()
        {
            return "on_menu_action";
        }

    public:
        MenuActionEvent(MenuActionType type, MenuActionArgs *args);

    public:
        void *get_args() override;
    };

    struct SetVarBitArgs
    {
        uint32_t world_setting_id;
        uint32_t value;
    };

    class SetVarBitEvent : public Event
    {
    public:
        SetVarBitArgs args;

    public:
        static constexpr ::std::string specific_id()
        {
            return "on_set_varbit";
        }

    public:
        SetVarBitEvent(uint32_t world_setting_id, uint32_t value);

    public:
        void *get_args() override;
    };
}
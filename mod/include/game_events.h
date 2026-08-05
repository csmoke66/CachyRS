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

    struct MenuOpenedEventArgs
    {
        bool opened;
    };

    class MenuOpenedEvent : public Event
    {
    public:
        MenuOpenedEventArgs args;

    public:
        static constexpr ::std::string specific_id()
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
        static constexpr ::std::string pre_id()
        {
            return "on_menu_action_pre";
        }

        static constexpr ::std::string post_id()
        {
            return "on_menu_action_post";
        }

    public:
        MenuActionEvent(const std::string &id, MenuActionArgs *args, MenuActionTemplate **action_template);

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
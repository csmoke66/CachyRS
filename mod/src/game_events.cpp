#include "game_events.h"

namespace crs
{
    EngineTickEvent::EngineTickEvent() : Event(EngineTickEvent::specific_id())
    {
    }

    MenuActionEvent::MenuActionEvent(MenuActionType type, MenuActionArgs *args) : Event(MenuActionEvent::specific_id())
    {
        this->args.type = type;
        this->args.args = args;
    }

    void* MenuActionEvent::get_args()
    {
        return &args;
    }

    SetVarBitEvent::SetVarBitEvent(uint32_t world_setting_id, uint32_t value) : Event(SetVarBitEvent::specific_id())
    {
        this->args.world_setting_id = world_setting_id;
        this->args.value = value;
    }

    void* SetVarBitEvent::get_args()
    {
        return &args;
    }
}
#include "game_events.h"

namespace crs
{
    EngineTickEvent::EngineTickEvent(Engine* engine) : Event(EngineTickEvent::specific_id())
    {
        this->args.engine = engine;
    }

    void* EngineTickEvent::get_args()
    {
        return &args;
    }

    MenuOpenedEvent::MenuOpenedEvent(bool opened) : Event(MenuOpenedEvent::specific_id())
    {
        this->args.opened = opened;
    }

    void *MenuOpenedEvent::get_args()
    {
        return &args;
    }

    MenuActionEvent::MenuActionEvent(const std::string &id, MenuActionArgs *args, MenuActionTemplate **action_template) : Event(id)
    {
        this->args.args = args;
        this->args.action_template = action_template;
    }

    void *MenuActionEvent::get_args()
    {
        return &args;
    }

    SetVarBitEvent::SetVarBitEvent(uint32_t world_setting_id, uint32_t value) : Event(SetVarBitEvent::specific_id())
    {
        this->args.world_setting_id = world_setting_id;
        this->args.value = value;
    }

    void *SetVarBitEvent::get_args()
    {
        return &args;
    }

    ItemChangedEvent::ItemChangedEvent(uint32_t id, uint32_t slot, int32_t old_id, int32_t old_amount, int32_t new_id, int32_t new_amount, int32_t stack_delta) : Event(ItemChangedEvent::specific_id())
    {
        this->args.id = id;
        this->args.slot = slot;
        this->args.old_id = old_id;
        this->args.old_amount = old_amount;
        this->args.new_id = new_id;
        this->args.new_amount = new_amount;
        this->args.stack_delta = stack_delta;
    }

    void *ItemChangedEvent::get_args()
    {
        return &args;
    }
}
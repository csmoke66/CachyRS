#include "event_bus.h"

namespace crs
{
    EngineTickEvent::EngineTickEvent() : Event(EngineTickEvent::specific_id())
    {
    }
    
    MenuActionEvent::MenuActionEvent(MenuActionType type, MenuActionArgs* args) : Event(MenuActionEvent::specific_id())
    {
        this->type = type;
        this->args = args;
    }
}
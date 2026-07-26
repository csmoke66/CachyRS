#include "event_bus.h"

namespace crs
{
    Event::Event(const ::std::string &id)
    {
        this->id = id;
    }

    void* Event::get_args()
    {
        return nullptr;
    }

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

    EventBusLane::EventBusLane()
    {
    }

    EventBusLane::EventBusLane(const ::std::string &id)
    {
        this->id = id;
    }

    EventBusLane::EventBusLane(const EventBusLane &o)
    {
        this->id = o.id;
    }

    void EventBusLane::add_receiver(EventReceiver<Event> *receiver)
    {
        event_receivers.push_back(receiver);
    }

    void EventBusLane::dispatch(Event *event)
    {
        for (auto &er : event_receivers)
        {
            er->receive(event);
        }
    }

    void EventBus::add_receiver(const ::std::string &id, EventReceiver<Event> *receiver)
    {
        auto it = lanes.find(id);
        if (it != lanes.end())
        {
            it->second.add_receiver(receiver);
        }
        else
        {
            lanes[id] = EventBusLane(id);
            lanes[id].add_receiver(receiver);
        }
    }

    void EventBus::dispatch(const ::std::string &id, Event *event)
    {
        auto it = lanes.find(id);
        if (it != lanes.end())
        {
            it->second.dispatch(event);
        }
    }
}
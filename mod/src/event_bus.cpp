#include "event_bus.h"
#include "game_events.h"

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
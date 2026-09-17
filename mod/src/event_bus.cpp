#include "event_bus.h"
#include "game_events.h"

namespace crs
{
  Event::Event(std::string_view id) : id(id)
  {
  }

  std::string_view Event::get_id() const
  {
    return id;
  }

  void *Event::get_args()
  {
    return nullptr;
  }

  EventBusLane::EventBusLane() = default;

  EventBusLane::EventBusLane(std::string id) : id(std::move(id))
  {
  }

  std::string_view EventBusLane::get_id() const
  {
    return id;
  }

  void EventBusLane::add_receiver(EventReceiver<Event> *receiver)
  {
    event_receivers.push_back(receiver);
  }

  void EventBusLane::dispatch(Event *event)
  {
    for (auto *er : event_receivers)
    {
      er->receive(event);
    }
  }

  void EventBus::add_receiver(std::string_view id, EventReceiver<Event> *receiver)
  {
    auto it = lanes.find(id);
    if (it != lanes.end())
    {
      it->second.add_receiver(receiver);
    }
    else
    {
      auto [lane_it, inserted] = lanes.emplace(std::string(id), EventBusLane(std::string(id)));
      lane_it->second.add_receiver(receiver);
    }
  }

  void EventBus::dispatch(std::string_view id, Event *event)
  {
    auto it = lanes.find(id);
    if (it != lanes.end())
    {
      it->second.dispatch(event);
    }
  }
} // namespace crs

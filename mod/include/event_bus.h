#pragma once
#include <concepts>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

#include <reversed/reversed.h>

namespace crs
{
  struct StringHash
  {
    using is_transparent = void;

    size_t operator()(std::string_view s) const noexcept
    {
      return std::hash<std::string_view>{}(s);
    }

    size_t operator()(const std::string &s) const noexcept
    {
      return std::hash<std::string>{}(s);
    }

    size_t operator()(const char *s) const noexcept
    {
      return std::hash<std::string_view>{}(s);
    }
  };

  class Event
  {
  private:
    std::string_view id;

  public:
    Event(std::string_view id);
    virtual ~Event() = default;

  public:
    std::string_view get_id() const;

  public:
    virtual void *get_args();
  };

  template <std::derived_from<Event> T>
  class EventReceiver
  {
  public:
    virtual void receive(T *event) = 0;
    virtual ~EventReceiver() = default;
  };

  class EventBusLane
  {
  private:
    std::string id;
    std::vector<EventReceiver<Event> *> event_receivers;

  public:
    EventBusLane();
    EventBusLane(std::string id);
    EventBusLane(const EventBusLane &o) = default;
    EventBusLane &operator=(const EventBusLane &o) = default;

  public:
    void add_receiver(EventReceiver<Event> *receiver);

  public:
    void dispatch(Event *event);

  public:
    std::string_view get_id() const;
  };

  class EventBus
  {
  private:
    std::unordered_map<std::string, EventBusLane, StringHash, std::equal_to<>> lanes;

  public:
    void add_receiver(std::string_view id, EventReceiver<Event> *receiver);

  public:
    void dispatch(std::string_view id, Event *event);
  };
} // namespace crs

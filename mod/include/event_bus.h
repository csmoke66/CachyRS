#pragma once
#include <vector>
#include <map>
#include <concepts>
#include <string>

#include <reversed/reversed.h>

namespace crs
{
    class Event
    {
    private:
        ::std::string id;

    public:
        Event(const ::std::string &id);

    public:
        ::std::string get_id();
    };

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

    class MenuActionEvent : public Event
    {
    public:
        MenuActionType type;
        MenuActionArgs* args;
        
    public:
        static constexpr ::std::string specific_id()
        {
            return "on_menu_action";
        }

    public:
        MenuActionEvent(MenuActionType type, MenuActionArgs* args);
    };

    template <std::derived_from<Event> T>
    class EventReceiver
    {
    public:
        virtual void receive(T *event) = 0;
    };

    class EventBusLane
    {
    private:
        ::std::string id;
        ::std::vector<EventReceiver<Event> *> event_receivers;

    public:
        EventBusLane(::std::string id);
        EventBusLane(EventBusLane &o);

    public:
        void add_receiver(EventReceiver<Event> *receiver);

    public:
        ::std::string get_id();
    };

    class EventBus
    {
    private:
        ::std::map<::std::string, EventBusLane> lanes;

    public:
        void add_receiver(const ::std::string &id, EventReceiver<Event> *receiver);

    public:
        void dispatch(const ::std::string &id, Event *event);
    };
}
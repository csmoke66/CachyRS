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

    public:
        virtual void* get_args();
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
        EventBusLane();
        EventBusLane(const ::std::string &id);
        EventBusLane(const EventBusLane &o);

    public:
        void add_receiver(EventReceiver<Event> *receiver);

    public:
        void dispatch(Event *event);

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
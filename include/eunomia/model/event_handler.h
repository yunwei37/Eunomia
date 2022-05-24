#ifndef EVENT_HANDLER_H
#define EVENT_HANDLER_H

#include <optional>

template <typename T>
struct tracker_event
{
    T data;
};

template <typename T>
struct event_handler_base
{
public:
    virtual ~event_handler_base() = default;
    virtual void handle(tracker_event<T> &e) = 0;
    virtual void do_handle_event(tracker_event<T> &e) = 0;
};

template <typename T>
struct event_handler : event_handler_base<T>
{
std::shared_ptr<event_handler_base<T>> next_handler = nullptr;
public:
    virtual ~event_handler() = default;
    virtual void handle(tracker_event<T> &e) = 0;
    std::shared_ptr<event_handler<T>> add_handler(std::shared_ptr<event_handler<T>> handler)
    {
        next_handler = handler;
        return handler;
    }
    void do_handle_event(tracker_event<T> &e)
    {
        handle(e);
        if (next_handler)
            next_handler->do_handle_event(e);
        return;
    }
};

template <typename T1, typename T2>
struct event_handler_adapter : event_handler_base<T2>
{
std::shared_ptr<event_handler_base<T2>> next_handler = nullptr;
public:
    virtual ~event_handler_adapter() = default;
    virtual tracker_event<T1> adapt(tracker_event<T2> &e) = 0;
    std::shared_ptr<event_handler<T1>> add_handler(std::shared_ptr<event_handler<T1>> handler)
    {
        next_handler = handler;
        return handler;
    }
    void do_handle_event(tracker_event<T2> &e)
    {
        auto event1 = adapt(e);
        if (next_handler)
            next_handler->do_handle_event(event1);
        return;
    }
};

#endif
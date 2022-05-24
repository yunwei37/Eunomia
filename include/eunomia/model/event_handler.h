#ifndef EVENT_HANDLER_H
#define EVENT_HANDLER_H

#include <optional>

template <typename T>
struct event
{
    T data;
};

template <typename T>
struct event_handler
{
    //std::optional<event_handler> next = std::nullopt;
    virtual void handle(event<T> &e) = 0;
    struct event_handler& add_handler(event_handler &handler)
    {
        //next = handler;
        return handler;
    }
};


#endif
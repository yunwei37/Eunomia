#ifndef TRACKER_CONFIG_H
#define TRACKER_CONFIG_H

#include <mutex>
#include <thread>
#include "event_handler.h"

template <typename ENV, typename EVENT>
struct tracker_config
{   
    ENV env;
    std::string name;
    //std::optional<event_handler<EVENT>> handler = std::nullopt;;
};

#endif
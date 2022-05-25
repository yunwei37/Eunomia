#ifndef TRACKER_CONFIG_H
#define TRACKER_CONFIG_H

#include <mutex>
#include <thread>
#include "event_handler.h"

// config data for tracker
template <typename ENV, typename EVENT>
struct tracker_config
{   
    ENV env;
    std::string name;
    std::shared_ptr<event_handler<EVENT>> handler = nullptr;
};

#endif
#include "eunomia/tracker_integrations.h"

extern "C"
{
#include "memleak/memleak_tracker.h"
}

std::unique_ptr<memleak_tracker> memleak_tracker::create_tracker_with_default_env(tracker_event_handler handler)
{
  config_data config;
  config.handler = handler;
  config.name = "memleak";
  config.env = tracker_alone_env{ .main_func = start_memleak };
  return std::make_unique<memleak_tracker>(config);
}

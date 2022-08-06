#include "eunomia/tracker_integrations.h"

extern "C"
{
#include "opensnoop/opensnoop_tracker.h"
}

std::unique_ptr<opensnoop_tracker> opensnoop_tracker::create_tracker_with_default_env(tracker_event_handler handler)
{
  config_data config;
  config.handler = handler;
  config.name = "opensnoop";
  config.env = tracker_alone_env{ .main_func = start_opensnoop };
  return std::make_unique<opensnoop_tracker>(config);
}

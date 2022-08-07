#include "eunomia/tracker_integrations.h"

extern "C"
{
#include "mountsnoop/mountsnoop_tracker.h"
}

std::unique_ptr<mountsnoop_tracker> mountsnoop_tracker::create_tracker_with_default_env(tracker_event_handler handler)
{
  config_data config;
  config.handler = handler;
  config.name = "mountsnoop";
  config.env = tracker_alone_env{ .main_func = start_mountsnoop };
  return std::make_unique<mountsnoop_tracker>(config);
}

std::unique_ptr<mountsnoop_tracker> mountsnoop_tracker::create_tracker_with_args(
    tracker_event_handler handler,
    const std::vector<std::string> &args)
{
  auto tracker = mountsnoop_tracker::create_tracker_with_default_env(handler);
  if (tracker)
  {
    tracker->current_config.env.process_args = args;
  }
  return tracker;
}

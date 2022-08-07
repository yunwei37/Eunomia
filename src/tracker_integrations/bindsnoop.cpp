#include "eunomia/tracker_integrations.h"

extern "C"
{
#include "bindsnoop/bindsnoop_tracker.h"
}

std::unique_ptr<bindsnoop_tracker> bindsnoop_tracker::create_tracker_with_default_env(tracker_event_handler handler)
{
  config_data config;
  config.handler = handler;
  config.name = "bindsnoop";
  config.env = tracker_alone_env{ .main_func = start_bindsnoop };
  return std::make_unique<bindsnoop_tracker>(config);
}

std::unique_ptr<bindsnoop_tracker> bindsnoop_tracker::create_tracker_with_args(
    tracker_event_handler handler,
    const std::vector<std::string> &args)
{
  auto tracker = bindsnoop_tracker::create_tracker_with_default_env(handler);
  if (tracker)
  {
    tracker->current_config.env.process_args = args;
  }
  return tracker;
}

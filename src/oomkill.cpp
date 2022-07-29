#include "eunomia/oomkill.h"

oomkill_tracker::oomkill_tracker(config_data config) : tracker_alone_base(config)
{
  exiting = false;
  this->current_config.env.exiting = &exiting;
}

std::unique_ptr<oomkill_tracker> oomkill_tracker::create_tracker_with_default_env(tracker_event_handler handler)
{
  config_data config;
  config.handler = handler;
  config.name = "oomkill_tracker";
  config.env = tracker_alone_env{ 0 };
  return std::make_unique<oomkill_tracker>(config);
}

oomkill_tracker::oomkill_tracker(tracker_alone_env env)
    : oomkill_tracker(config_data{
          .env = env,
      })
{
}

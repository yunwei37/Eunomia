#include "eunomia/tracker_integrations.h"

namespace oom_tracker_space {
  extern "C"
  {
  #include "oomkill/oom_tracker.h"
  }
}

std::unique_ptr<oomkill_tracker> oomkill_tracker::create_tracker_with_default_env(tracker_event_handler handler)
{
  config_data config;
  config.handler = handler;
  config.name = "oomkill";
  config.env = tracker_alone_env{ .main_func = oom_tracker_space::start_oomkill };
  return std::make_unique<oomkill_tracker>(config);
}

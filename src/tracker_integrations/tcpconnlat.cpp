#include "eunomia/tracker_integrations.h"

namespace tcpconnlat_tracker_space {
  extern "C"
  {
  #include "tcpconnlat/tcpconnlat_tracker.h"
  }
}

std::unique_ptr<tcpconnlat_tracker> tcpconnlat_tracker::create_tracker_with_default_env(tracker_event_handler handler)
{
  config_data config;
  config.handler = handler;
  config.name = "tcpconnlat";
  config.env = tracker_alone_env{ .main_func = tcpconnlat_tracker_space::start_tcpconnlat };
  return std::make_unique<tcpconnlat_tracker>(config);
}

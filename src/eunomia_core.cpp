#include "eunomia/eunomia_core.h"

#include <iostream>

#include "eunomia/container.h"
#include "eunomia/tracker_manager.h"

eunomia_core::eunomia_core(config& config) : core_config(config), core_prometheus_server(config.prometheus_listening_address)
{
}

template<tracker_concept TRACKER>
TRACKER::tracker_event_handler eunomia_core::create_tracker_event_handler()
{
  typename TRACKER::tracker_event_handler handler = nullptr;

  for (auto e : core_config.enabled_export_types)
  {
    typename TRACKER::tracker_event_handler new_handler = nullptr;
    switch (e)
    {
      case export_type::prometheus:
        new_handler = std::make_shared<typename TRACKER::prometheus_event_handler>(
            typename TRACKER::prometheus_event_handler(core_prometheus_server));
      break;
      case export_type::stdout_json:
        new_handler = std::make_shared<typename TRACKER::json_event_printer>(typename TRACKER::json_event_printer{});
       break;
      default: std::cout << "unsupported export type\n"; break;
    }
    if (new_handler)
    {
      if (handler)
      {
        handler->add_handler(new_handler);
      }
      else
      {
        handler = new_handler;
      }
    }
  }
  return handler;
}

template<tracker_concept TRACKER>
std::unique_ptr<TRACKER> eunomia_core::create_default_tracker(const tracker_data_base* base)
{
  using cur_tracker_data = const tracker_data<typename TRACKER::config_data>;
  cur_tracker_data* data;
  if (!base || !(data = (cur_tracker_data*)(base)))
  {
    std::cout << "start_tracker got wrong type data\n";
    return nullptr;
  }
  const cur_tracker_data& files_data = *data;

  auto handler = create_tracker_event_handler<TRACKER>();
  if (!handler) {
    std::cout << "no handler was created for tracker\n";
    return nullptr;
  }
  auto tracker_ptr = TRACKER::create_tracker_with_default_env(handler);

  return tracker_ptr;
}

void eunomia_core::start_trackers(void)
{
  for (auto t : core_config.enabled_trackers)
  {
    std::cout << "start ebpf tracker...\n";
    if (!t)
    {
      std::cout << "tracker data is null\n";
      continue;
    }
    switch (t->type)
    {
      case avaliable_tracker::tcp: break;
      case avaliable_tracker::syscall: break;
      case avaliable_tracker::ipc: break;
      case avaliable_tracker::process: break;
      case avaliable_tracker::files: 
          core_tracker_manager.start_tracker(create_default_tracker<files_tracker>(t.get()));
        break;
      default: 
        std::cout << "unsupported tracker type\n";
      break;
    }
  }
}

int eunomia_core::start_eunomia(void)
{
  return 0;
}
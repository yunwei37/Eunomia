/* SPDX-License-Identifier: GPL-2.0 OR BSD-3-Clause
 *
 * Copyright (c) 2022, 郑昱笙，濮雯旭，张典典（牛校牛子队）
 * All rights reserved.
 */

#include "eunomia/eunomia_core.h"

#include <spdlog/spdlog.h>

#include "eunomia/container.h"
#include "eunomia/tracker_manager.h"

eunomia_core::eunomia_core(config& config) : core_config(config), core_prometheus_server(config.prometheus_listening_address)
{
}

// create event handler for print to console
template<tracker_concept TRACKER>
TRACKER::tracker_event_handler eunomia_core::create_print_event_handler(const TRACKER* tracker_ptr)
{
  switch (core_config.fmt)
  {
    case export_format::json_format: return std::make_shared<typename TRACKER::json_event_printer>();
    case export_format::plain_text: return std::make_shared<typename TRACKER::plain_text_event_printer>();
    case export_format::csv: return std::make_shared<typename TRACKER::csv_event_printer>();
    case export_format::none: return nullptr;
    default: spdlog::error("unsupported output format to stdout"); return nullptr;
  }
  return nullptr;
}

// create all event handlers for a tracker
template<tracker_concept TRACKER>
TRACKER::tracker_event_handler eunomia_core::create_tracker_event_handler(const TRACKER* tracker_ptr)
{
  typename TRACKER::tracker_event_handler handler = nullptr;

  // create event handler based on export types
  for (auto e : core_config.enabled_export_types)
  {
    typename TRACKER::tracker_event_handler new_handler = nullptr;
    switch (e)
    {
      case export_type::prometheus:
        new_handler = std::make_shared<typename TRACKER::prometheus_event_handler>(
            typename TRACKER::prometheus_event_handler(core_prometheus_server));
        break;
      case export_type::stdout_type: new_handler = create_print_event_handler<TRACKER>(tracker_ptr); break;
      default: spdlog::error("unsupported export type."); break;
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
    spdlog::error("start_tracker got wrong type data");
    return nullptr;
  }
  const cur_tracker_data& files_data = *data;

  auto handler = create_tracker_event_handler<TRACKER>(nullptr);
  if (!handler)
  {
    spdlog::error("no handler was created for tracker");
    return nullptr;
  }
  auto tracker_ptr = TRACKER::create_tracker_with_default_env(handler);

  return tracker_ptr;
}

void eunomia_core::start_trackers(void)
{
  for (auto t : core_config.enabled_trackers)
  {
    spdlog::info("start ebpf tracker...");
    if (!t)
    {
      spdlog::error("tracker data is null");
      continue;
    }
    switch (t->type)
    {
      case avaliable_tracker::tcp:
        spdlog::info("tcp tracker is starting");
        core_tracker_manager.start_tracker(create_default_tracker<tcp_tracker>(t.get()));
        break;
      case avaliable_tracker::syscall:
        spdlog::info("syscall tracker is starting");
        core_tracker_manager.start_tracker(create_default_tracker<syscall_tracker>(t.get()));
        break;
      case avaliable_tracker::ipc:
        spdlog::info("ipc tracker is starting");
        core_tracker_manager.start_tracker(create_default_tracker<ipc_tracker>(t.get()));
        break;
      case avaliable_tracker::process:
        spdlog::info("process tracker is starting");
        core_tracker_manager.start_tracker(create_default_tracker<process_tracker>(t.get()));
        break;
      case avaliable_tracker::files:
        spdlog::info("files tracker is starting");
        core_tracker_manager.start_tracker(create_default_tracker<files_tracker>(t.get()));
        break;
      default: std::cout << "unsupported tracker type\n"; break;
    }
  }
}

int eunomia_core::start_eunomia(void)
{
  spdlog::info("start eunomia...");
  try
  {
    start_trackers();
  }
  catch (const std::exception& e)
  {
    spdlog::error("eunomia start tracker failed: {}", e.what());
    return 1;
  }

  if (core_config.enable_container_manager)
  {
    spdlog::info("start container manager...");
    core_container_manager.start_container_tracing();
  }
  if (core_config.enabled_export_types.count(export_type::prometheus))
  {
    spdlog::info("start prometheus server...");
    core_prometheus_server.start_prometheus_server();
  }
  if (core_config.is_auto_exit)
  {
    spdlog::info("set exit time...");
    std::this_thread::sleep_for(std::chrono::seconds(core_config.exit_after));
    spdlog::info("auto exit...");
  }
  else
  {
    spdlog::info("press 'Ctrl C' key to exit...");
    while (std::cin.get() != 'x')
    {
      std::this_thread::sleep_for(std::chrono::seconds(1));
    }
  }
  return 0;
}

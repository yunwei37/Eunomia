/* SPDX-License-Identifier: GPL-2.0 OR BSD-3-Clause
 *
 * Copyright (c) 2022, 郑昱笙，濮雯旭，张典典（牛校牛子队）
 * All rights reserved.
 */

#include "eunomia/eunomia_core.h"

#include <spdlog/spdlog.h>

#include "eunomia/sec_analyzer.h"
#include "eunomia/tracker_manager.h"

eunomia_core::eunomia_core(eunomia_config_data& config)
    : core_config(config),
      core_prometheus_server(config.prometheus_listening_address)
{
  core_config.load_config_options_to_trackers();
}

template<tracker_concept TRACKER>
TRACKER::tracker_event_handler eunomia_core::create_tracker_event_handler(const handler_config_data& config)
{
  if (config.name == "json_format")
  {
    return std::make_shared<typename TRACKER::json_event_printer>();
  }
  else if (config.name == "plain_text")
  {
    return std::make_shared<typename TRACKER::plain_text_event_printer>();
  }
  else if (config.name == "csv")
  {
    return std::make_shared<typename TRACKER::csv_event_printer>();
  }
  else if (config.name == "prometheus")
  {
    return std::make_shared<typename TRACKER::prometheus_event_handler>(
        typename TRACKER::prometheus_event_handler(core_prometheus_server));
  }
  else if (config.name == "none")
  {
    return nullptr;
  }
  else
  {
    spdlog::error("unsupported event handler {}", config.name);
    return nullptr;
  }
}

template<tracker_concept TRACKER>
TRACKER::tracker_event_handler eunomia_core::create_tracker_event_handlers(
    const std::vector<handler_config_data>& handler_configs)
{
  typename TRACKER::tracker_event_handler handler = nullptr;
  for (auto& config : handler_configs)
  {
    auto new_handler = create_tracker_event_handler<TRACKER>(config);
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

// create a default tracker with other handlers
template<tracker_concept TRACKER>
std::unique_ptr<TRACKER> eunomia_core::create_default_tracker_with_handler(
    const tracker_config_data& base,
    TRACKER::tracker_event_handler additional_handler)
{
  auto handler = create_tracker_event_handlers<TRACKER>(base.export_handlers);
  if (!handler)
  {
    spdlog::error("no handler was created for tracker");
    return nullptr;
  }
  if (additional_handler)
  {
    additional_handler->add_handler(handler);
    handler = additional_handler;
  }
  auto tracker_ptr = TRACKER::create_tracker_with_default_env(handler);

  return tracker_ptr;
}

template<tracker_concept TRACKER>
std::unique_ptr<TRACKER> eunomia_core::create_default_tracker(const tracker_config_data& base)
{
  return create_default_tracker_with_handler<TRACKER>(base, nullptr);
}

template<tracker_concept TRACKER, typename SEC_ANALYZER_HANDLER>
std::unique_ptr<TRACKER> eunomia_core::create_default_tracker_with_sec_analyzer(const tracker_config_data& base)
{
  std::shared_ptr<SEC_ANALYZER_HANDLER> handler = nullptr;
  if (core_config.enable_sec_rule_detect)
  {
    handler = std::make_shared<SEC_ANALYZER_HANDLER>(core_sec_analyzer);
  }
  return create_default_tracker_with_handler<TRACKER>(base, handler);
}

void eunomia_core::start_tracker(const tracker_config_data& config)
{
  if (config.name == "files")
  {
    core_tracker_manager.start_tracker(create_default_tracker<files_tracker>(config));
  }
  else if (config.name == "process")
  {
    core_tracker_manager.start_tracker(create_default_tracker<process_tracker>(config));
  }
  else if (config.name == "syscall")
  {
    create_default_tracker_with_sec_analyzer<syscall_tracker, syscall_rule_checker>(config);
  }
  else if (config.name == "tcpconnect")
  {
    core_tracker_manager.start_tracker(create_default_tracker<tcp_tracker>(config));
  }
  else
  {
    spdlog::error("unknown tracker name: {}", config.name);
  }
  spdlog::info("{} tracker is started", config.name);
}

void eunomia_core::start_trackers(void)
{
  for (auto& t : core_config.enabled_trackers)
  {
    spdlog::info("start ebpf tracker...");
    start_tracker(t);
  }
}

void eunomia_core::check_auto_exit(void)
{
  if (core_config.is_auto_exit)
  {
    spdlog::info("set exit time...");
    std::this_thread::sleep_for(std::chrono::seconds(core_config.exit_after));
    spdlog::info("auto exit...");
    exit(0);
  }
  else
  {
    spdlog::info("press 'Ctrl C' key to exit...");
    while (std::cin.get() != 'x')
    {
      std::this_thread::sleep_for(std::chrono::seconds(1));
    }
  }
}

void eunomia_core::start_prometheus_server(void)
{
  if (core_config.enabled_export_types.count("prometheus"))
  {
    spdlog::info("start prometheus server...");
    core_prometheus_server.start_prometheus_server();
  }
}

void eunomia_core::start_sec_analyzer(void)
{
  if (core_config.enable_sec_rule_detect)
  {
    spdlog::info("start safe module...");
    if (core_config.enabled_export_types.count("prometheus"))
    {
      core_sec_analyzer = sec_analyzer_prometheus::create_sec_analyzer_with_default_rules(core_prometheus_server);
    }
    else
    {
      core_sec_analyzer = sec_analyzer::create_sec_analyzer_with_default_rules();
    }
  }
}

void eunomia_core::start_container_manager(void)
{
  if (core_config.enable_container_manager)
  {
    spdlog::error("start container manager...not yet");
    // core_container_manager.start_container_tracing(core_config.container_log_path);
  }
}

int eunomia_core::start_eunomia(void)
{
  spdlog::info("start eunomia...");
  try
  {
    start_sec_analyzer();
    start_container_manager();
    start_trackers();
    start_prometheus_server();
    check_auto_exit();
  }
  catch (const std::exception& e)
  {
    spdlog::error("eunomia start failed: {}", e.what());
    return 1;
  }
  return 0;
}

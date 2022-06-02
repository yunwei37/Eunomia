/* SPDX-License-Identifier: GPL-2.0 OR BSD-3-Clause
 *
 * Copyright (c) 2022, 郑昱笙，濮雯旭，张典典（牛校牛子队）
 * All rights reserved.
 */

#include "eunomia/sec_analyzer.h"

#include <spdlog/spdlog.h>

sec_analyzer_prometheus::sec_analyzer_prometheus(prometheus_server &server, const std::vector<sec_rule_describe> &rules)
    : eunomia_sec_warn_counter(prometheus::BuildCounter()
                                   .Name("eunomia_seccurity_warn_count")
                                   .Help("Number of observed security warnings")
                                   .Register(*server.registry)),
      eunomia_sec_event_counter(prometheus::BuildCounter()
                                    .Name("eunomia_seccurity_event_count")
                                    .Help("Number of observed security event")
                                    .Register(*server.registry)),
      eunomia_sec_alert_counter(prometheus::BuildCounter()
                                    .Name("eunomia_seccurity_alert_count")
                                    .Help("Number of observed security alert")
                                    .Register(*server.registry)),
      sec_analyzer(rules)
{
}

std::string sec_rule_level_string(sec_rule_level level)
{
  switch (level)
  {
    case sec_rule_level::warnning: return "warnning";
    case sec_rule_level::event: return "event";
    case sec_rule_level::alert: return "alert";
    default: return "unknown";
  }
}

void sec_analyzer::print_event(const rule_message &msg)
{
  spdlog::info("{}", "Security Rule Detection:");
  spdlog::info("level: {}", sec_rule_level_string(msg.level));
  spdlog::info("name: {}", msg.name);
  spdlog::info("message: {}", msg.message);
  spdlog::info("pid: {}", msg.pid);
  spdlog::info("container_id: {}", msg.container_id);
  spdlog::info("container_name: {}", msg.container_name);
}

void sec_analyzer::report_event(const rule_message &msg)
{
  print_event(msg);
}

void sec_analyzer_prometheus::report_event(const rule_message &msg)
{
  print_event(msg);
  report_prometheus_event(msg);
}

void sec_analyzer_prometheus::report_prometheus_event(const struct rule_message &msg)
{
  switch (msg.level)
  {
    case sec_rule_level::event:
      eunomia_sec_event_counter
          .Add({ { "level", "event" },
                 { "name", msg.name },
                 { "message", msg.message },
                 { "pid", std::to_string(msg.pid) },
                 { "container_id", msg.container_id },
                 { "container_name", msg.container_name } })
          .Increment();
      break;
    case sec_rule_level::warnning:
      eunomia_sec_warn_counter
          .Add({ { "level", "warning" },
                 { "name", msg.name },
                 { "message", msg.message },
                 { "pid", std::to_string(msg.pid) },
                 { "container_id", msg.container_id },
                 { "container_name", msg.container_name } })
          .Increment();
      break;
    case sec_rule_level::alert:
      eunomia_sec_alert_counter
          .Add({ { "level", "alert" },
                 { "name", msg.name },
                 { "message", msg.message },
                 { "pid", std::to_string(msg.pid) },
                 { "container_id", msg.container_id },
                 { "container_name", msg.container_name } })
          .Increment();
      break;
    default: break;
  }
}

int syscall_rule::check_rule(const syscall_event &e)
{
  if (!analyzer)
  {
    return -1;
  }
  for (auto i = 0; i < analyzer->rules.size(); i++)
  {
    if (analyzer->rules[i].name == syscall_names_x86_64[e.syscall_id])
    {
      return i;
    }
  }
  return -1;
}
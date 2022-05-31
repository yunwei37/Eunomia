/* SPDX-License-Identifier: GPL-2.0 OR BSD-3-Clause
 *
 * Copyright (c) 2022, 郑昱笙，濮雯旭，张典典（牛校牛子队）
 * All rights reserved.
 */

#include "eunomia/process.h"

#include <spdlog/spdlog.h>

extern "C"
{
#include <process/process_tracker.h>
}

void process_tracker::prometheus_event_handler::report_prometheus_event(const struct process_event &e)
{
  // TODO: fix this
  std::string ids[] = { "36fca8c5eec1", "e2055f599ca6" };
  std::string names[] = { "Ubuntu", "Debian" };
  size_t n = (size_t)std::rand() % 2;
  if (e.exit_event)
  {
    eunomia_process_exit_counter
        .Add({ { "exit_code", std::to_string(e.exit_code) },
               { "duration_ms", std::to_string(e.duration_ns / 1000000) },
               { "comm", std::string(e.comm) },
               //  // TODO: fix container part
               { "container_name", names[n] },
               { "container_id", ids[n] },
               { "pid", std::to_string(e.common.pid) } })
        .Increment();
  }
  else
  {
    eunomia_process_start_counter
        .Add({ { "comm", std::string(e.comm) },
               { "filename", std::string(e.filename) },
               // TODO: fix container part
               { "container_name", names[n] },
               { "container_id", ids[n] },
               { "pid", std::to_string(e.common.pid) } })
        .Increment();
  }
}

process_tracker::prometheus_event_handler::prometheus_event_handler(prometheus_server &server)
    : eunomia_process_start_counter(prometheus::BuildCounter()
                                        .Name("eunomia_bserved_process_start")
                                        .Help("Number of observed process start")
                                        .Register(*server.registry)),
      eunomia_process_exit_counter(prometheus::BuildCounter()
                                       .Name("eunomia_observed_process_end")
                                       .Help("Number of observed process start")
                                       .Register(*server.registry))
{
}

void process_tracker::prometheus_event_handler::handle(tracker_event<process_event> &e)
{
  report_prometheus_event(e.data);
}

process_tracker::process_tracker(config_data config) : tracker_with_config(config)
{
  exiting = false;
  this->current_config.env.exiting = &exiting;
}

std::unique_ptr<process_tracker> process_tracker::create_tracker_with_default_env(tracker_event_handler handler)
{
  config_data config;
  config.handler = handler;
  config.name = "process_tracker";
  config.env = process_env{
    //.min_duration_ms = 20,
  };
  return std::make_unique<process_tracker>(config);
}

process_tracker::process_tracker(process_env env)
    : process_tracker(config_data{
          .env = env,
      })
{
}

void process_tracker::start_tracker()
{
  struct process_bpf *skel = nullptr;
  // start_process_tracker(handle_event, libbpf_print_fn, current_config.env, skel, (void *)this);
  start_process_tracker(
      handle_tracker_event<process_tracker, process_event>, libbpf_print_fn, current_config.env, skel, (void *)this);
}

std::string process_tracker::json_event_handler_base::to_json(const struct process_event &e)
{
  std::string res;
  json process_event = { { "type", "process" },
                         { "time", get_current_time() },
                         { "pid", e.common.pid },
                         { "ppid", e.common.ppid },
                         { "cgroup_id", e.common.cgroup_id },
                         { "user_namespace_id", e.common.user_namespace_id },
                         { "pid_namespace_id", e.common.pid_namespace_id },
                         { "mount_namespace_id", e.common.mount_namespace_id },
                         { "exit_code", e.exit_code },
                         { "duration_ns", e.duration_ns },
                         { "comm", e.comm },
                         { "filename", e.filename },
                         { "exit_event", e.exit_event } };
  return process_event.dump();
}

void process_tracker::json_event_printer::handle(tracker_event<process_event> &e)
{
  std::cout << to_json(e.data) << std::endl;
}

void process_tracker::plain_text_event_printer::handle(tracker_event<process_event> &e)
{
  static bool is_start = true;
  if (is_start)
  {
    is_start = false;
    spdlog::info(
        "{}\t{}\t{}\t{}\t{}\t{}\t{}\t{}",
        "pid",
        "ppid",
        "cgroup_id",
        "user_namespace_id",
        "pid_namespace_id",
        "mount_namespace_id",
        "exit_code/comm",
        "duration_ns/filename");
  }
  if (e.data.exit_event)
  {
    spdlog::info(
        "{}\t{}\t{}\t{}\t{}\t{}\t{}\t{}",
        e.data.common.pid,
        e.data.common.ppid,
        e.data.common.cgroup_id,
        e.data.common.user_namespace_id,
        e.data.common.pid_namespace_id,
        e.data.common.mount_namespace_id,
        e.data.exit_code,
        e.data.duration_ns);
    return;
  }
  spdlog::info(
      "{}\t{}\t{}\t{}\t{}\t{}\t{}\t{}",
      e.data.common.pid,
      e.data.common.ppid,
      e.data.common.cgroup_id,
      e.data.common.user_namespace_id,
      e.data.common.pid_namespace_id,
      e.data.common.mount_namespace_id,
      e.data.comm,
      e.data.filename);
}
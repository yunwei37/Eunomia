/* SPDX-License-Identifier: GPL-2.0 OR BSD-3-Clause
 *
 * Copyright (c) 2022, 郑昱笙，濮雯旭，张典典（牛校牛子队）
 * All rights reserved.
 */

#include "eunomia/ipc.h"

#include <spdlog/spdlog.h>

ipc_tracker::ipc_tracker(config_data config) : tracker_with_config(config)
{
  exiting = false;
  this->current_config.env.exiting = &exiting;
}

std::unique_ptr<ipc_tracker> ipc_tracker::create_tracker_with_default_env(tracker_event_handler handler)
{
  config_data config;
  config.handler = handler;
  config.name = "ipc_tracker";
  config.env = ipc_env{ 0 };
  return std::make_unique<ipc_tracker>(config);
}

ipc_tracker::ipc_tracker(ipc_env env)
    : ipc_tracker(config_data{
          .env = env,
      })
{
}

void ipc_tracker::start_tracker()
{
  // current_config.env.ctx = (void *)this;
  start_ipc_tracker(handle_tracker_event<ipc_tracker, ipc_event>, libbpf_print_fn, current_config.env);
}

json ipc_tracker::json_event_handler::to_json(const struct ipc_event &e)
{
  std::string res;
  json ipc = { { "type", "ipc" }, { "time", get_current_time() } };
  json ipc_event_json = json::array();

  ipc_event_json.push_back({
      { "pid", e.pid },
      { "uid", e.uid },
      { "gid", e.gid },
      { "cuid", e.cuid },
      { "cgid", e.cgid },
  });
  ipc.push_back({ "ipc", ipc_event_json });
  return ipc;
}

void ipc_tracker::json_event_printer::handle(tracker_event<ipc_event> &e)
{
  std::cout << to_json(e.data).dump() << std::endl;
}

void ipc_tracker::plain_text_event_printer::handle(tracker_event<ipc_event> &e)
{
  static bool is_start = true;
  if (is_start)
  {
    is_start = false;
    spdlog::info("pid\tuid\tgid\tcuid\tcgid");
  }

  spdlog::info("{}\t{}\t\t{}\t\t{}\t\t{}", e.data.pid, e.data.uid, e.data.gid, e.data.cuid, e.data.cgid);
}

void ipc_tracker::prometheus_event_handler::report_prometheus_event(const struct ipc_event &e)
{
  // eunomia_ipc_write_counter
  //     .Add({ { "type", std::to_string(e.values[i].type) },
  //            { "filename", std::string(e.values[i].filename) },
  //            { "comm", std::string(e.values[i].comm) },
  //            { "pid", std::to_string(e.values[i].pid) } })
  //     .Increment((double)e.values[i].writes);
  // eunomia_ipc_read_counter
  //     .Add({
  //         { "comm", std::string(e.values[i].comm) },
  //         { "filename", std::string(e.values[i].filename) },
  //         { "pid", std::to_string(e.values[i].pid) },
  //         { "type", std::to_string(e.values[i].type) },
  //     })
  //     .Increment((double)e.values[i].reads);
  // eunomia_ipc_write_bytes
  //     .Add({ { "type", std::to_string(e.values[i].type) },
  //            { "filename", std::string(e.values[i].filename) },
  //            { "comm", std::string(e.values[i].comm) },
  //            { "pid", std::to_string(e.values[i].pid) } })
  //     .Increment((double)e.values[i].write_bytes);
  // eunomia_ipc_read_bytes
  //     .Add({
  //         { "comm", std::string(e.values[i].comm) },
  //         { "filename", std::string(e.values[i].filename) },
  //         { "pid", std::to_string(e.values[i].pid) },
  //         { "type", std::to_string(e.values[i].type) },
  //     })
  //     .Increment((double)e.values[i].read_bytes);
}

ipc_tracker::prometheus_event_handler::prometheus_event_handler(prometheus_server &server)
// : eunomia_ipc_read_counter(prometheus::BuildCounter()
//                                  .Name("eunomia_observed_ipc_read_count")
//                                  .Help("Number of observed ipc read count")
//                                  .Register(*server.registry)),
//   eunomia_ipc_write_counter(prometheus::BuildCounter()
//                                   .Name("eunomia_observed_ipc_write_count")
//                                   .Help("Number of observed ipc write count")
//                                   .Register(*server.registry)),
//   eunomia_ipc_write_bytes(prometheus::BuildCounter()
//                                 .Name("eunomia_observed_ipc_write_bytes")
//                                 .Help("Number of observed ipc write bytes")
//                                 .Register(*server.registry)),
//   eunomia_ipc_read_bytes(prometheus::BuildCounter()
//                                .Name("eunomia_observed_ipc_read_bytes")
//                                .Help("Number of observed ipc read bytes")
//                                .Register(*server.registry))
{
}

void ipc_tracker::prometheus_event_handler::handle(tracker_event<ipc_event> &e)
{
  report_prometheus_event(e.data);
}

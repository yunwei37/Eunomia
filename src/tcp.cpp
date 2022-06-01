/* SPDX-License-Identifier: GPL-2.0 OR BSD-3-Clause
 *
 * Copyright (c) 2022, 郑昱笙，濮雯旭，张典典（牛校牛子队）
 * All rights reserved.
 */

#include "eunomia/tcp.h"

#include <spdlog/spdlog.h>

#include <json.hpp>
#include <string>

using json = nlohmann::json;

tcp_tracker::tcp_tracker(config_data config) : tracker_with_config(config)
{
  exiting = false;
  this->current_config.env.exiting = &exiting;
}

std::unique_ptr<tcp_tracker> tcp_tracker::create_tracker_with_default_env(tracker_event_handler handler)
{
  config_data config;
  config.handler = handler;
  config.name = "tcp_tracker";
  config.env = tcp_env{ 0 };
  return std::make_unique<tcp_tracker>(config);
}

void tcp_tracker::start_tracker()
{
  // current_config.env.ctx = (void *)this;
  start_tcp_tracker(handle_tcp_sample_event, libbpf_print_fn, current_config.env);
}

int tcp_tracker::fill_src_dst(sender &s, sender &d, const tcp_event &e)
{
  if (e.af == AF_INET)
  {
    s.x4.s_addr = e.saddr_v4;
    d.x4.s_addr = e.daddr_v4;
  }
  else if (e.af == AF_INET6)
  {
    memcpy(&s.x6.s6_addr, e.saddr_v6, sizeof(s.x6.s6_addr));
    memcpy(&d.x6.s6_addr, e.daddr_v6, sizeof(d.x6.s6_addr));
  }
  else
  {
    fprintf(stderr, "broken tcp_event: tcp_event->af=%d", e.af);
    return -1;
  }
  return 0;
}

std::string tcp_tracker::json_event_handler_base::to_json(const struct tcp_event &e)
{
  char src[INET6_ADDRSTRLEN];
  char dst[INET6_ADDRSTRLEN];
  sender s, d;
  if (tcp_tracker::fill_src_dst(s, d, e) < 0)
  {
    spdlog::debug("broken tcp_event\n");
  }

  json tcp = { { "type", "tcp" },
               { "time", get_current_time() },
               { "pid", e.pid },
               { "task", e.task },
               { "af", AF_INET ? 4 : 6 },
               { "src", inet_ntop((int)e.af, &s, src, sizeof(src)) },
               { "dst", inet_ntop((int)e.af, &d, dst, sizeof(dst)) },
               { "dport", ntohs(e.dport) } };

  return tcp.dump();
}

void tcp_tracker::json_event_printer::handle(tracker_event<tcp_event> &e)
{
  std::cout << to_json(e.data) << std::endl;
}

void tcp_tracker::plain_text_event_printer::handle(tracker_event<tcp_event> &e)
{
  static bool is_start = true;
  if (is_start)
  {
    is_start = false;
    spdlog::info("pid\ttask\taf\tsrc\tdst\tdport");
  }
  char src[INET6_ADDRSTRLEN];
  char dst[INET6_ADDRSTRLEN];
  sender s, d;
  if (tcp_tracker::fill_src_dst(s, d, e.data) < 0)
  {
    spdlog::debug("broken tcp_event\n");
  }

  spdlog::info(
      "{}\t{}\t\t{}\t\t{}\t\t{}\t\t{}",
      e.data.pid,
      e.data.task,
      AF_INET ? 4 : 6,
      inet_ntop((int)e.data.af, &s, src, sizeof(src)),
      inet_ntop((int)e.data.af, &d, dst, sizeof(dst)),
      ntohs(e.data.dport));
}

void tcp_tracker::csv_event_printer::handle(tracker_event<tcp_event> &e)
{
  static bool is_start = true;
  if (is_start)
  {
    is_start = false;
    spdlog::info("pid,task,af,src,dst,dport");
  }
  char src[INET6_ADDRSTRLEN];
  char dst[INET6_ADDRSTRLEN];
  sender s, d;
  if (tcp_tracker::fill_src_dst(s, d, e.data) < 0)
  {
    spdlog::debug("broken tcp_event\n");
  }

  spdlog::info(
      "{},{},{},{},{},{}",
      e.data.pid,
      e.data.task,
      AF_INET ? 4 : 6,
      inet_ntop((int)e.data.af, &s, src, sizeof(src)),
      inet_ntop((int)e.data.af, &d, dst, sizeof(dst)),
      ntohs(e.data.dport));
}

void tcp_tracker::prometheus_event_handler::report_prometheus_event(const struct tcp_event &e)
{
  // eunomia_tcp_write_counter
  //     .Add({ { "type", std::to_string(e.values[i].type) },
  //            { "filename", std::string(e.values[i].filename) },
  //            { "comm", std::string(e.values[i].comm) },
  //            { "pid", std::to_string(e.values[i].pid) } })
  //     .Increment((double)e.values[i].writes);
  // eunomia_tcp_read_counter
  //     .Add({
  //         { "comm", std::string(e.values[i].comm) },
  //         { "filename", std::string(e.values[i].filename) },
  //         { "pid", std::to_string(e.values[i].pid) },
  //         { "type", std::to_string(e.values[i].type) },
  //     })
  //     .Increment((double)e.values[i].reads);
  // eunomia_tcp_write_bytes
  //     .Add({ { "type", std::to_string(e.values[i].type) },
  //            { "filename", std::string(e.values[i].filename) },
  //            { "comm", std::string(e.values[i].comm) },
  //            { "pid", std::to_string(e.values[i].pid) } })
  //     .Increment((double)e.values[i].write_bytes);
  // eunomia_tcp_read_bytes
  //     .Add({
  //         { "comm", std::string(e.values[i].comm) },
  //         { "filename", std::string(e.values[i].filename) },
  //         { "pid", std::to_string(e.values[i].pid) },
  //         { "type", std::to_string(e.values[i].type) },
  //     })
  //     .Increment((double)e.values[i].read_bytes);
}

tcp_tracker::prometheus_event_handler::prometheus_event_handler(prometheus_server &server)
// : eunomia_tcp_read_counter(prometheus::BuildCounter()
//                                  .Name("eunomia_observed_tcp_read_count")
//                                  .Help("Number of observed tcp read count")
//                                  .Register(*server.registry)),
//   eunomia_tcp_write_counter(prometheus::BuildCounter()
//                                   .Name("eunomia_observed_tcp_write_count")
//                                   .Help("Number of observed tcp write count")
//                                   .Register(*server.registry)),
//   eunomia_tcp_write_bytes(prometheus::BuildCounter()
//                                 .Name("eunomia_observed_tcp_write_bytes")
//                                 .Help("Number of observed tcp write bytes")
//                                 .Register(*server.registry)),
//   eunomia_tcp_read_bytes(prometheus::BuildCounter()
//                                .Name("eunomia_observed_tcp_read_bytes")
//                                .Help("Number of observed tcp read bytes")
//                                .Register(*server.registry))
{
}

void tcp_tracker::prometheus_event_handler::handle(tracker_event<tcp_event> &e)
{
  report_prometheus_event(e.data);
}

void tcp_tracker::handle_tcp_sample_event(void *ctx, int cpu, void *data, unsigned int data_sz)
{
  handle_tracker_event<tcp_tracker, tcp_event>(ctx, data, (size_t)data_sz);
}

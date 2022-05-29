#include "eunomia/tcp.h"

#include <spdlog/spdlog.h>

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

json tcp_tracker::json_event_handler_base::to_json(const struct tcp_event &e)
{
  json tcp = { { "type", "tcp" }, { "time", get_current_time() } 
  //TODO: add more fields
  };
  
  return tcp;
}

void tcp_tracker::json_event_printer::handle(tracker_event<tcp_event> &e)
{
  std::cout << to_json(e.data).dump() << std::endl;
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
  union
  {
    struct in_addr x4;
    struct in6_addr x6;
  } s, d;
  if (e.data.af == AF_INET)
  {
    s.x4.s_addr = e.data.saddr_v4;
    d.x4.s_addr = e.data.daddr_v4;
  }
  else if (e.data.af == AF_INET6)
  {
    memcpy(&s.x6.s6_addr, e.data.saddr_v6, sizeof(s.x6.s6_addr));
    memcpy(&d.x6.s6_addr, e.data.daddr_v6, sizeof(d.x6.s6_addr));
  }
  else
  {
    fprintf(stderr, "broken tcp_event: tcp_event->af=%d", e.data.af);
    return;
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

#include "eunomia/process.h"

extern "C"
{
#include <process/process_tracker.h>
}

void process_tracker::prometheus_event_handler::report_prometheus_event(const struct process_event &e)
{
  if (e.exit_event)
  {
    process_exit_counter
        .Add({ { "exit_code", std::to_string(e.exit_code) },
               { "duration_ms", std::to_string(e.duration_ns / 1000000) },
               { "comm", std::string(e.comm) },
               // { "pid", std::to_string(e.common.pid) } 
               })
        .Increment();
  }
  else
  {
    process_start_counter
        .Add({ { "comm", std::string(e.comm) },
               { "filename", std::string(e.filename) },
               // { "pid", std::to_string(e.common.pid) } 
               })
        .Increment();
  }
}

process_tracker::prometheus_event_handler::prometheus_event_handler(prometheus_server &server)
    : process_start_counter(prometheus::BuildCounter()
                                .Name("observed_process_start")
                                .Help("Number of observed process start")
                                .Register(*server.registry)),
      process_exit_counter(prometheus::BuildCounter()
                               .Name("observed_process_end")
                               .Help("Number of observed process start")
                               .Register(*server.registry))
{
}

void process_tracker::prometheus_event_handler::handle(tracker_event<process_event> &e)
{
  report_prometheus_event(e.data);
}

process_tracker::process_tracker(process_config config) : tracker_with_config(config)
{
  exiting = false;
  this->current_config.env.exiting = &exiting;
}

process_tracker::process_tracker(process_env env) : process_tracker(process_config{
      .env = env,
  })
{
}

void process_tracker::start_tracker()
{
  struct process_bpf *skel = nullptr;
  //start_process_tracker(handle_event, libbpf_print_fn, current_config.env, skel, (void *)this);
  start_process_tracker(handle_tracker_event<process_tracker, process_event>, libbpf_print_fn, current_config.env, skel, (void *)this);
}

std::string process_tracker::json_event_handler::to_json(const struct process_event &e)
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

/*
int process_tracker::handle_event(void *ctx, void *data, size_t data_sz)
{
  if (!data || !ctx)
  {
    return -1;
  }
  const struct process_event &e = *(const struct process_event *)data;
  process_tracker &pt = *(process_tracker *)ctx;
  auto event = tracker_event{ e };
  if (pt.current_config.handler)
  {
    pt.current_config.handler->do_handle_event(event);
  }
  return 0;
}
*/

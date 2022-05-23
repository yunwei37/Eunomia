#include "eunomia/process.h"

extern "C"
{
#include <process/process_tracker.h>
}

process_tracker::process_tracker(process_env env, prometheus_server &server)
    : env(env),
      process_start_counter(prometheus::BuildCounter()
                                .Name("observed_process_start")
                                .Help("Number of observed process start")
                                .Register(*server.registry)),
      process_exit_counter(prometheus::BuildCounter()
                               .Name("observed_process_end")
                               .Help("Number of observed process start")
                               .Register(*server.registry))
{
  exiting = false;
  this->env.exiting = &exiting;
}

void process_tracker::start_tracker()
{
  struct process_bpf *skel = nullptr;
  start_process_tracker(handle_event, libbpf_print_fn, env, skel, (void *)this);
}

void process_tracker::report_prometheus_event(const struct process_event &e)
{
  if (e.exit_event)
  {
    process_exit_counter
        .Add({ { "exit_code", std::to_string(e.exit_code) },
               { "duration_ms", std::to_string(e.duration_ns / 1000000) },
               { "comm", std::string(e.comm) },
               { "pid", std::to_string(e.common.pid) } })
        .Increment();
  }
  else
  {
    process_start_counter
        .Add({
               { "comm", std::string(e.comm) },
               { "filename", std::string(e.filename) },
               { "pid", std::to_string(e.common.pid) } })
        .Increment();
  }
}

std::string process_tracker::to_json(const struct process_event &e)
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

int process_tracker::handle_event(void *ctx, void *data, size_t data_sz)
{
  if (!data || !ctx)
  {
    return -1;
  }
  const struct process_event &e = *(const struct process_event *)data;
  process_tracker &pt = *(process_tracker *)ctx;
  std::cout << to_json(e) << std::endl;
  pt.report_prometheus_event(e);
  return 0;
}
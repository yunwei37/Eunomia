#include "eunomia/process.h"

extern "C"
{
#include <process/process_tracker.h>
}

process_tracker::process_tracker(process_env env, prometheus::Family<prometheus::Counter> &counter)
    : env(env),
      process_start_counter(counter.Add({ { "process_event", "start" } })),
      process_exit_counter(counter.Add({ { "process_event", "exit" } }))
{
  exiting = false;
  this->env.exiting = &exiting;
}

void process_tracker::start_tracker()
{
  struct process_bpf *skel = nullptr;
  start_process_tracker(handle_event, libbpf_print_fn, env, skel, (void *)this);
}

void report_prometheus_event(const struct process_event &e)
{
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
  if (!data)
  {
    return -1;
  }
  const struct process_event *e = (const struct process_event *)data;
  std::cout << to_json(*e) << std::endl;
  return 0;
}
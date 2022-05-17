#ifndef PROCESS_CMD_H
#define PROCESS_CMD_H

#include <iostream>
#include <jsoncons/json.hpp>
#include <jsoncons_ext/jsonpath/jsonpath.hpp>
#include <mutex>
#include <string>
#include <thread>

#include "libbpf_print.h"

extern "C" {
#include "process/process_tracker.h"
}

struct process_tracker {
  volatile bool exiting;
  std::mutex mutex;
  struct process_env env = {0};
  process_tracker() {
    env = {0};
    exiting = false;
    env.exiting = &exiting;
  }
  process_tracker(pid_t target_pid, long min_duration_ms) {
    exiting = false;
    env = {0};
    env.exiting = &exiting;
    env.target_pid = target_pid;
    env.min_duration_ms = min_duration_ms;
  }
  void start_process() {
    start_process_tracker(handle_event, libbpf_print_fn, env);
  }
  static std::string to_json(const struct process_event &e) {
    std::string res;
    jsoncons::json process_event(
        jsoncons::json_object_arg,
        {{"type", "process"},
         {"pid", e.common.pid},
         {"ppid", e.common.ppid},
         {"cgroup_id", e.common.cgroup_id},
         {"user_namespace_id", e.common.user_namespace_id},
         {"pid_namespace_id", e.common.pid_namespace_id},
         {"mount_namespace_id", e.common.mount_namespace_id},
         {"exit_code", e.exit_code},
         {"duration_ns", e.duration_ns},
         {"comm", e.comm},
         {"filename", e.filename},
         {"exit_event", e.exit_event}});
    process_event.dump(res);
    return res;
  }
  static int handle_event(void *ctx, void *data, size_t data_sz) {
    if (!data) {
      return -1;
    }
    const struct process_event *e = (const struct process_event *)data;
    std::cout << to_json(*e) << std::endl;
    return 0;
  }
};

#endif
#ifndef PROCESS_CMD_H
#define PROCESS_CMD_H

#include <iostream>
#include <json.hpp>
#include <mutex>
#include <string>
#include <thread>

#include "libbpf_print.h"
#include "tracker.h"

extern "C" {
#include <process/process_tracker.h>
}

using json = nlohmann::json;

struct process_tracker : public tracker {
  struct process_env env = {0};
  process_tracker(process_env env) : env(env) {
    exiting = false;
    this->env.exiting = &exiting;
  }
  void start_tracker() {
    struct process_bpf *skel;
    start_process_tracker(handle_event, libbpf_print_fn, env, skel);
  }
  static std::string to_json(const struct process_event &e) {
    std::string res;
    json process_event = {{"type", "process"},
                          {"time", get_current_time()},
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
                          {"exit_event", e.exit_event}};
    return process_event.dump();
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
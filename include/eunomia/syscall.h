#ifndef SYSCALL_CMD_H
#define SYSCALL_CMD_H

#include <iostream>
#include <mutex>
#include <thread>

#include "libbpf_print.h"
#include "tracker.h"

extern "C" {
#include <syscall/syscall_tracker.h>
#include "syscall_helper.h"
}

struct syscall_tracker : public tracker {
  struct syscall_env current_env = {0};
  syscall_tracker(syscall_env env) {
    exiting = false;
    this->current_env = env;
    this->current_env.exiting = &exiting;
  }
  syscall_tracker() {
    current_env = {0};
    exiting = false;
    current_env.exiting = &exiting;
  }
  void start_tracker(void) override {
    start_syscall_tracker(handle_event, libbpf_print_fn, current_env);
  }
  static std::string to_json(const struct syscall_event &e) {
    json syscall_event = {{"type", "syscall"},
                          {"time", get_current_time()},
                          {"pid", e.pid},
                          {"ppid", e.ppid},
                          {"mount_namespace_id", e.mntns},
                          {"syscall_id", e.syscall_id},
                          {"comm", e.comm},
                          {"occur_times", e.occur_times}};
    return syscall_event.dump();
  }
  static void print_event(const struct syscall_event *e) {
    auto time = get_current_time();

    if (e->syscall_id >= syscall_names_x86_64_size)
      return;

    printf("%-8s %-16s %-7d %-7d [%lu] %u\t%s\t%d\n", time.c_str(), e->comm,
           e->pid, e->ppid, e->mntns, e->syscall_id,
           syscall_names_x86_64[e->syscall_id], e->occur_times);
  }
  static int handle_event(void *ctx, void *data, size_t data_sz) {
    const struct syscall_event *e = (const struct syscall_event *)data;
    if (!e) {
      return -1;
    }
    print_event(e);
    return 0;
  }
};

#endif
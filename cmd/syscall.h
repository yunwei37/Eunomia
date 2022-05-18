#ifndef SYSCALL_CMD_H
#define SYSCALL_CMD_H

#include <iostream>
#include <mutex>
#include <thread>

#include "libbpf_print.h"

extern "C" {
#include "syscall/syscall_tracker.h"
#include "syscall/syscall_helper.h"
}

struct syscall_tracker {
  volatile bool exiting;
  std::mutex mutex;
  struct syscall_env env = {0};
  syscall_tracker() {
    env = {0};
    exiting = false;
    env.exiting = &exiting;
  }
  syscall_tracker(pid_t target_pid, long min_duration_ms) {
    exiting = false;
    env = {0};
    env.exiting = &exiting;
    env.target_pid = target_pid;
    env.min_duration_ms = min_duration_ms;
  }
  void start_syscall() {
    start_syscall_tracker(handle_event, libbpf_print_fn, env);
  }
  static std::string to_json(const struct syscall_event &e) {
    json syscall_event = {{"type", "syscall"},
                          {"time", get_current_time()},
                          {"pid", e.pid},
                          {"ppid", e.ppid},
                          {"mount_namespace_id", e.mntns},
                          };
    return syscall_event.dump();
  }
  static void print_event(const struct process_event *e) {
    auto time = get_current_time();

    printf("%-8s %-16s %-7d %-7d [%lu] %u\t%s\n", time.c_str(), e->comm, e->pid,
           e->ppid, e->mntns, e->syscall_id, syscall_names_x86_64[e->syscall_id]);
  }
  static int handle_event(void *ctx, void *data, size_t data_sz) {
    const struct syscall_event *e = (const struct syscall_event *)data;
    
    return 0;
  }
};

#endif
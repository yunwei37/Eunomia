#ifndef CONTAINER_CMD_H
#define CONTAINER_CMD_H

#include <iostream>
#include <json.hpp>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <stdio.h>
#include <thread>
#include <mutex>
#include <memory>

#include "libbpf_print.h"
#include "tracker.h"

extern "C" {
#include <container/container.h>
#include <process/process_tracker.h>
#include <unistd.h>
}

using json = nlohmann::json;
static std::unordered_map<int, struct container_event> container_processes;
static std::mutex mp_lock;

struct container_tracker : public tracker {
  struct process_env env = {0};
  container_tracker(process_env env);
  void start_tracker();

  static void fill_event(struct process_event &event);

  static void init_container_table();

  static void print_container(const struct container_event &e);

  static void judge_container(const struct process_event &e);

  static int handle_event(void *ctx, void *data, size_t data_sz);

  static unsigned long get_container_id_via_pid(pid_t pid);

  static std::unordered_map<int, struct container_event> get_map();
};

#endif
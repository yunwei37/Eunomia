/* SPDX-License-Identifier: GPL-2.0 OR BSD-3-Clause
 *
 * Copyright (c) 2022, 郑昱笙，濮雯旭，张典典（牛校牛子队）
 * All rights reserved.
 */

#ifndef CONTAINER_CMD_H
#define CONTAINER_CMD_H

#include <stdio.h>

#include <iostream>
#include <json.hpp>
#include <memory>
#include <mutex>
#include <thread>
#include <unordered_map>
#include <unordered_set>
#include <vector>

extern "C"
{
#include <container/container.h>
#include <process/process_tracker.h>
#include <unistd.h>
}

#include "libbpf_print.h"
#include "model/tracker.h"
#include "tracker_manager.h"

struct container_env
{
  struct process_env penv;
  // other fields: TODO
  bool print_result;
};

struct container_tracker : public tracker_with_config<container_env, container_event>
{
  struct container_env current_env = { 0 };
  struct container_manager &this_manager;

  container_tracker(container_env env, container_manager &manager);
  void start_tracker();

  void fill_event(struct process_event &event);

  void init_container_table();

  void print_container(const struct container_event &e);

  void judge_container(const struct process_event &e);

  static int handle_event(void *ctx, void *data, size_t data_sz);
};

struct container_manager
{
 private:
  struct tracker_manager tracker;
  std::mutex mp_lock;
  std::unordered_map<int, struct container_event> container_processes;
  friend struct container_tracker;

 public:
  void start_container_tracing()
  { 
    tracker.start_tracker(std::make_unique<container_tracker>(container_env{
      .print_result = true,
    }, *this));
  }
  unsigned long get_container_id_via_pid(pid_t pid);
};

#endif

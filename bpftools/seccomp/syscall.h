/* SPDX-License-Identifier: GPL-2.0 OR BSD-3-Clause
 *
 * Copyright (c) 2022, 郑昱笙，濮雯旭，张典典（牛校牛子队）
 * All rights reserved.
 * 
 */

#ifndef SYSCALL_CMD_H
#define SYSCALL_CMD_H

#include <json.hpp>
#include <mutex>
#include <thread>

#include "libbpf_print.h"
#include "model/tracker.h"
#include "prometheus/counter.h"
#include "prometheus_server.h"

extern "C" {
#include <syscall/syscall_tracker.h>
#include "syscall_helper.h"
}

using json = nlohmann::json;

struct syscall_tracker : public tracker_with_config<syscall_env, syscall_event> {
  syscall_tracker(config_data config);

  // create a tracker with deafult config
  static std::unique_ptr<syscall_tracker> create_tracker_with_default_env(tracker_event_handler handler);

  syscall_tracker(syscall_env env);

  void start_tracker();

  // used for prometheus exporter
  struct prometheus_event_handler : public event_handler<syscall_event>
  { 
    // // read times counter for field reads
    // prometheus::Family<prometheus::Counter> &eunomia_files_read_counter;
    // // write times counter for field writes
    // prometheus::Family<prometheus::Counter> &eunomia_files_write_counter;
    // // write bytes counter for field write_bytes
    // prometheus::Family<prometheus::Counter> &eunomia_files_write_bytes;
    // // read bytes counter for field read_bytes
    // prometheus::Family<prometheus::Counter> &eunomia_files_read_bytes;
    void report_prometheus_event(const struct syscall_event &e);

    prometheus_event_handler(prometheus_server &server);
    void handle(tracker_event<syscall_event> &e);
  };

  // convert event to json
  struct json_event_handler : public event_handler<syscall_event>
  {
    json to_json(const struct syscall_event &e);
  };

  // used for json exporter, inherits from json_event_handler
  struct json_event_printer : public json_event_handler
  {
    void handle(tracker_event<syscall_event> &e);
  };

  struct plain_text_event_printer : public event_handler<syscall_event>
  {
    void handle(tracker_event<syscall_event> &e);
  };
};

#endif
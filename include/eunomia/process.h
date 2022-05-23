#ifndef PROCESS_CMD_H
#define PROCESS_CMD_H

#include <iostream>
#include <json.hpp>
#include <mutex>
#include <string>
#include <thread>

#include "libbpf_print.h"
#include "tracker.h"
#include "prometheus/counter.h"
#include "prometheus_server.h"

using json = nlohmann::json;

struct process_tracker : public tracker {
  struct process_env env = {0};
  
  prometheus::Counter & process_start_counter;
  prometheus::Counter & process_exit_counter;

  process_tracker(process_env env, prometheus::Family<prometheus::Counter> &counter);
  void start_tracker();
  void add_prometheus_counter(prometheus_server& server);
  void report_prometheus_event(const struct process_event &e);

  static std::string to_json(const struct process_event &e);
  static int handle_event(void *ctx, void *data, size_t data_sz);
};

#endif
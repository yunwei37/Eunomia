#ifndef PROCESS_CMD_H
#define PROCESS_CMD_H

#include <iostream>
#include <json.hpp>
#include <mutex>
#include <string>
#include <thread>

#include "libbpf_print.h"
#include "prometheus/counter.h"
#include "prometheus_server.h"
#include "model/tracker.h"

using json = nlohmann::json;

struct process_tracker : public tracker_with_config<process_env, process_event>
{
  struct process_env current_env = { 0 };

  prometheus::Family<prometheus::Counter> &process_start_counter;
  prometheus::Family<prometheus::Counter> &process_exit_counter;

  process_tracker(process_env env, prometheus_server &server);
  void start_tracker();
  void report_prometheus_event(const struct process_event &e);

  static std::string to_json(const struct process_event &e);
  static int handle_event(void *ctx, void *data, size_t data_sz);
};

#endif
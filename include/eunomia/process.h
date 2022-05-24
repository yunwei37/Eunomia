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
  using process_config = tracker_config<process_env, process_event>;
  process_tracker(process_config config);
  process_tracker(process_env env);
  void start_tracker();
  
  static int handle_event(void *ctx, void *data, size_t data_sz);

  struct prometheus_event_handler: public event_handler<process_event>
  {
    prometheus::Family<prometheus::Counter> &process_start_counter;
    prometheus::Family<prometheus::Counter> &process_exit_counter;
    void report_prometheus_event(const struct process_event &e);

    prometheus_event_handler(prometheus_server &server);
    void handle(tracker_event<process_event> &e);
  };

  struct json_event_handler: public event_handler<process_event>
  {
    std::string to_json(const struct process_event &e);
  };

  struct json_event_printer: public json_event_handler {
    void handle(tracker_event<process_event> &e);
  };

};

#endif
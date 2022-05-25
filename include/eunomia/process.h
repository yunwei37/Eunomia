#ifndef PROCESS_CMD_H
#define PROCESS_CMD_H

#include <iostream>
#include <json.hpp>
#include <mutex>
#include <string>
#include <thread>

#include "libbpf_print.h"
#include "model/tracker.h"
#include "prometheus/counter.h"
#include "prometheus_server.h"

using json = nlohmann::json;

struct process_tracker : public tracker_with_config<process_env, process_event>
{
  using process_config = tracker_config<process_env, process_event>;
  using process_event_handler = std::shared_ptr<event_handler<process_event>>;

  process_tracker(process_config config);

  // create a tracker with deafult config
  static std::unique_ptr<process_tracker> create_tracker_with_default_env(process_event_handler handler);

  process_tracker(process_env env);
  // start process tracker
  void start_tracker();

  struct prometheus_event_handler : public event_handler<process_event>
  {
    prometheus::Family<prometheus::Counter> &eunomia_process_start_counter;
    prometheus::Family<prometheus::Counter> &eunomia_process_exit_counter;
    void report_prometheus_event(const struct process_event &e);

    prometheus_event_handler(prometheus_server &server);
    void handle(tracker_event<process_event> &e);
  };

  struct json_event_handler : public event_handler<process_event>
  {
    std::string to_json(const struct process_event &e);
  };

  struct json_event_printer : public json_event_handler
  {
    void handle(tracker_event<process_event> &e);
  };
};

#endif
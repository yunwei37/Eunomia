#ifndef FILE_CMD_H
#define FILE_CMD_H

#include <iostream>
#include <json.hpp>
#include <mutex>
#include <string>
#include <thread>

#include "libbpf_print.h"
#include "model/tracker.h"
#include "prometheus/counter.h"
#include "prometheus_server.h"
extern "C"
{
#include <files/file_tracker.h>
}

using json = nlohmann::json;

struct files_tracker : public tracker_with_config<files_env, files_event>
{
  using files_config = tracker_config<files_env, files_event>;
  using files_event_handler = std::shared_ptr<event_handler<files_event>>;

  files_tracker(files_config config);

  // create a tracker with deafult config
  static std::unique_ptr<files_tracker> create_tracker_with_default_env(files_event_handler handler);

  files_tracker(files_env env);
  // start files tracker
  void start_tracker();

  struct prometheus_event_handler : public event_handler<files_event>
  {
    prometheus::Family<prometheus::Counter> &eunomia_files_read_counter;
    prometheus::Family<prometheus::Counter> &eunomia_files_write_counter;
    prometheus::Family<prometheus::Counter> &eunomia_files_write_bytes;
    prometheus::Family<prometheus::Counter> &eunomia_files_read_bytes;
    void report_prometheus_event(const struct files_event &e);

    prometheus_event_handler(prometheus_server &server);
    void handle(tracker_event<files_event> &e);
  };

  struct json_event_handler : public event_handler<files_event>
  {
    json to_json(const struct files_event &e);
  };

  struct json_event_printer : public json_event_handler
  {
    void handle(tracker_event<files_event> &e);
  };
};

#endif
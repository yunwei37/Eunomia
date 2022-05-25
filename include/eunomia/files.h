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
  using config_data = tracker_config<files_env, files_event>;
  using tracker_event_handler = std::shared_ptr<event_handler<files_event>>;

  files_tracker(config_data config);

  // create a tracker with deafult config
  static std::unique_ptr<files_tracker> create_tracker_with_default_env(tracker_event_handler handler);

  files_tracker(files_env env);
  // start files tracker
  void start_tracker();

  // used for prometheus exporter
  struct prometheus_event_handler : public event_handler<files_event>
  { 
    // read times counter for field reads
    prometheus::Family<prometheus::Counter> &eunomia_files_read_counter;
    // write times counter for field writes
    prometheus::Family<prometheus::Counter> &eunomia_files_write_counter;
    // write bytes counter for field write_bytes
    prometheus::Family<prometheus::Counter> &eunomia_files_write_bytes;
    // read bytes counter for field read_bytes
    prometheus::Family<prometheus::Counter> &eunomia_files_read_bytes;
    void report_prometheus_event(const struct files_event &e);

    prometheus_event_handler(prometheus_server &server);
    void handle(tracker_event<files_event> &e);
  };

  // convert event to json
  struct json_event_handler : public event_handler<files_event>
  {
    json to_json(const struct files_event &e);
  };

  // used for json exporter, inherits from json_event_handler
  struct json_event_printer : public json_event_handler
  {
    void handle(tracker_event<files_event> &e);
  };
};

#endif
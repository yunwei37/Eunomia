/* SPDX-License-Identifier: GPL-2.0 OR BSD-3-Clause
 *
 * Copyright (c) 2022, 郑昱笙，濮雯旭，张典典（牛校牛子队）
 * All rights reserved.
 */

#include "eunomia/files.h"
#include <json.hpp>
#include <spdlog/spdlog.h>

using json = nlohmann::json;

void files_tracker::prometheus_event_handler::report_prometheus_event(const struct files_event &e)
{
  for (int i = 0; i < e.rows; i++)
  {
    eunomia_files_write_counter
        .Add({ { "type", std::to_string(e.values[i].type) },
               { "filename", std::string(e.values[i].filename) },
               { "comm", std::string(e.values[i].comm) },
               { "pid", std::to_string(e.values[i].pid) } })
        .Increment((double)e.values[i].writes);
    eunomia_files_read_counter
        .Add({
            { "comm", std::string(e.values[i].comm) },
            { "filename", std::string(e.values[i].filename) },
            { "pid", std::to_string(e.values[i].pid) },
            { "type", std::to_string(e.values[i].type) },
        })
        .Increment((double)e.values[i].reads);
    eunomia_files_write_bytes
        .Add({ { "type", std::to_string(e.values[i].type) },
               { "filename", std::string(e.values[i].filename) },
               { "comm", std::string(e.values[i].comm) },
               { "pid", std::to_string(e.values[i].pid) } })
        .Increment((double)e.values[i].write_bytes);
    eunomia_files_read_bytes
        .Add({
            { "comm", std::string(e.values[i].comm) },
            { "filename", std::string(e.values[i].filename) },
            { "pid", std::to_string(e.values[i].pid) },
            { "type", std::to_string(e.values[i].type) },
        })
        .Increment((double)e.values[i].read_bytes);
  }
}

files_tracker::prometheus_event_handler::prometheus_event_handler(prometheus_server &server)
    : eunomia_files_read_counter(prometheus::BuildCounter()
                                     .Name("eunomia_observed_files_read_count")
                                     .Help("Number of observed files read count")
                                     .Register(*server.registry)),
      eunomia_files_write_counter(prometheus::BuildCounter()
                                      .Name("eunomia_observed_files_write_count")
                                      .Help("Number of observed files write count")
                                      .Register(*server.registry)),
      eunomia_files_write_bytes(prometheus::BuildCounter()
                                    .Name("eunomia_observed_files_write_bytes")
                                    .Help("Number of observed files write bytes")
                                    .Register(*server.registry)),
      eunomia_files_read_bytes(prometheus::BuildCounter()
                                   .Name("eunomia_observed_files_read_bytes")
                                   .Help("Number of observed files read bytes")
                                   .Register(*server.registry))
{
}

void files_tracker::prometheus_event_handler::handle(tracker_event<files_event> &e)
{
  report_prometheus_event(e.data);
}

files_tracker::files_tracker(config_data config) : tracker_with_config(config)
{
  exiting = false;
  this->current_config.env.exiting = &exiting;
}

std::unique_ptr<files_tracker> files_tracker::create_tracker_with_default_env(tracker_event_handler handler)
{
  config_data config;
  config.handler = handler;
  config.name = "files_tracker";
  config.env = files_env{
    .target_pid = 0,
    .clear_screen = false,
    .regular_file_only = true,
    .output_rows = OUTPUT_ROWS_LIMIT,
    .sort_by = ALL,
    .interval = 3,
    .count = 99999999,
    .verbose = false,
  };
  return std::make_unique<files_tracker>(config);
}

files_tracker::files_tracker(files_env env)
    : files_tracker(config_data{
          .env = env,
      })
{
}

void files_tracker::start_tracker()
{
  struct files_bpf *skel = nullptr;
  // start_files_tracker(handle_event, libbpf_print_fn, current_config.env, skel, (void *)this);
  current_config.env.ctx = (void *)this;
  start_file_tracker(handle_tracker_event<files_tracker, files_event>, libbpf_print_fn, current_config.env);
}

std::string files_tracker::json_event_handler::to_json(const struct files_event &e)
{
  std::string res;
  json files = { { "type", "process" }, { "time", get_current_time() } };
  json files_event_json = json::array();
  for (int i = 0; i < e.rows; i++)
  {
    files_event_json.push_back({ { "pid", e.values[i].pid },
                                 { "read_bytes", e.values[i].read_bytes },
                                 { "reads", e.values[i].reads },
                                 { "write_bytes", e.values[i].write_bytes },
                                 { "writes", e.values[i].writes },
                                 { "comm", e.values[i].comm },
                                 { "filename", e.values[i].filename },
                                 { "type", e.values[i].type },
                                 { "tid", e.values[i].tid } });
  }
  files.push_back({ "files", files_event_json });
  return files.dump();
}

void files_tracker::json_event_printer::handle(tracker_event<files_event> &e)
{
  spdlog::info(to_json(e.data));
}

void files_tracker::plain_text_event_printer::handle(tracker_event<files_event> &e)
{
  static bool is_start = true;
  if (is_start)
  {
    is_start = false;
    spdlog::info("pid\tread_bytes\tread count\twrite_bytes\twrite count\tcomm\ttype\ttid\tfilename");
  }
  for (int i = 0; i < e.data.rows; i++)
  {
    spdlog::info(
        "{}\t{}\t\t{}\t\t{}\t\t{}\t\t{}\t{}\t{}\t{}",
        e.data.values[i].pid,
        e.data.values[i].read_bytes,
        e.data.values[i].reads,
        e.data.values[i].write_bytes,
        e.data.values[i].writes,
        e.data.values[i].comm,
        e.data.values[i].type,
        e.data.values[i].tid,
        e.data.values[i].filename);
  }
}

void files_tracker::csv_event_printer::handle(tracker_event<files_event> &e)
{
  static bool is_start = true;
  if (is_start)
  {
    is_start = false;
    spdlog::info("pid,read_bytes,read_count,write_bytes,write count,comm,type,tid,filename");
  }
  for (int i = 0; i < e.data.rows; i++)
  {
    spdlog::info(
        "{},{},{},{},{},{},{},{},{}",
        e.data.values[i].pid,
        e.data.values[i].read_bytes,
        e.data.values[i].reads,
        e.data.values[i].write_bytes,
        e.data.values[i].writes,
        e.data.values[i].comm,
        e.data.values[i].type,
        e.data.values[i].tid,
        e.data.values[i].filename);
  }
}
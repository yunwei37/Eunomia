#include "eunomia/files.h"

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

json files_tracker::json_event_handler::to_json(const struct files_event &e)
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
  return files;
}

void files_tracker::json_event_printer::handle(tracker_event<files_event> &e)
{
  std::cout << to_json(e.data).dump() << std::endl;
}
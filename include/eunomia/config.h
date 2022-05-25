#ifndef EUNOMIA_CONFIG_H
#define EUNOMIA_CONFIG_H

#include <chrono>
#include <map>
#include <set>
#include <string>
#include <vector>

#include "files.h"
#include "ipc.h"
#include "model/tracker_config.h"
#include "process.h"
#include "tcp.h"

// export config
enum class export_format
{
  json_format,
  csv,
  plant_text
};
enum class export_type
{
  prometheus,
  stdout_type,
  file,
  databse
};

enum class cmd_mode
{
  run,
  daemon,
  seccomp,
  server,
  help
};

enum class tracing_type
{
  pid,
  container_id,
  container_name,
  all
};

// all available tracker
enum class avaliable_tracker
{
  syscall,
  process,
  ipc,
  tcp,
  files
};

//
struct tracker_data_base
{
  avaliable_tracker type;
};

// tracker config data template
template<typename T>
struct tracker_data : tracker_data_base
{
  T config;
};

using process_tracker_data = tracker_data<process_tracker::config_data>;
using files_tracker_data = tracker_data<files_tracker::config_data>;

// config for eunomia
// both config from toml and command line should be put here
struct config
{
  cmd_mode run_selected;

  // config for all enabled tracker
  // each tracker should have its own config, for example, process_tracker_data
  // you can add fields to config_data, just replace the using with struct decleration
  // see:
  // using config_data = tracker_config<process_env, process_event>;
  std::vector<std::shared_ptr<tracker_data_base>> enabled_trackers;

  // export config
  // may be we should have config similar to tracker_config
  // TODO
  std::set<export_type> enabled_export_types;

  // export format
  // this should be set as well
  // TODO
  export_format fmt;

  // tracing config
  tracing_type tracing_selected;

  // tracing targets
  std::string container_name;
  unsigned long target_contaienr_id;
  pid_t target_pid;

  bool is_auto_exit = false;
  std::chrono::seconds exit_after;

  // TODO: this should be add to export config
  std::string prometheus_listening_address = "127.0.0.1:8528";
};

#endif
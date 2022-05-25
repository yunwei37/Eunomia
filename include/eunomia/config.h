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
  stdout_json,
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

struct config
{
  cmd_mode run_selected;

  std::vector<std::shared_ptr<tracker_data_base>> enabled_trackers;
  std::set<export_type> enabled_export_types;

  export_format fmt;
  tracing_type tracing_selected;

  std::string container_name;
  unsigned long target_contaienr_id;
  pid_t target_pid;

  bool is_auto_exit = false;
  std::chrono::seconds exit_after;

  std::string prometheus_listening_address = "127.0.0.1:8528";
};

#endif
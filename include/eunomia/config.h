/* SPDX-License-Identifier: GPL-2.0 OR BSD-3-Clause
 *
 * Copyright (c) 2022, 郑昱笙，濮雯旭，张典典（牛校牛子队）
 * All rights reserved.
 */

#ifndef EUNOMIA_CONFIG_H
#define EUNOMIA_CONFIG_H

#include <chrono>
#include <map>
#include <set>
#include <string>
#include <vector>
#include <string>

#include "files.h"
#include "ipc.h"
#include "model/tracker_config.h"
#include "process.h"
#include "tcp.h"
#include "syscall.h"

// export config
enum class export_format
{
  json_format,
  csv,
  plain_text,
  none,
};
static const std::vector<std::string> str_export_format = {"json", "csv", "plain_txt"};

enum class export_type
{
  prometheus,
  stdout_type,
  file,
  database
};
static const std::vector<std::string> str_export_type = {"prometheus", "stdout_type","file", "databse"};

enum class eunomia_mode
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
  all
};

// all available tracker
enum class avaliable_tracker
{
  syscall,
  process,
  ipc,
  tcp,
  files,
  help
};
static const std::vector<std::string> str_avaliable_tracker = {"syscall", "process", "ipc", "tcp", "files"};

// TODO refactor
// we should not use enum class
// use dynamic_cast to get the type of tracker
struct tracker_data_base
{
  avaliable_tracker type;
};

// tracker config data template
template<typename T>
struct tracker_data : tracker_data_base
{
  T config;
  tracker_data(avaliable_tracker t) : tracker_data_base{t} {}
};

using process_tracker_data = tracker_data<process_tracker::config_data>;
using files_tracker_data = tracker_data<files_tracker::config_data>;
using ipc_tracker_data = tracker_data<ipc_tracker::config_data>;
using syscall_tracker_data = tracker_data<syscall_tracker::config_data>;
using tcp_tracker_data = tracker_data<tcp_tracker::config_data>;

struct rule_config 
{
    std::string rule_name;
    std::string type;
    std::string trigger;
    std::string err_msg;
};

struct seccomp_config {
  // the syscalls name which is allowed
  std::vector<std::string> allow_syscall;
};

// config for eunomia
// both config from toml and command line should be put here
struct config
{
  // global run mode
  eunomia_mode run_selected = eunomia_mode::server;

  // config for all enabled tracker
  // each tracker should have its own config, for example, process_tracker_data
  // you can add fields to config_data, just replace the using with struct decleration
  // see:
  // using config_data = tracker_config<process_env, process_event>;
  std::vector<std::shared_ptr<tracker_data_base>> enabled_trackers = {
        std::make_shared<process_tracker_data>(avaliable_tracker::process),
        std::make_shared<files_tracker_data>(avaliable_tracker::files),
        std::make_shared<files_tracker_data>(avaliable_tracker::syscall),
        std::make_shared<files_tracker_data>(avaliable_tracker::tcp),
  };

  // export config
  // may be we should have config similar to tracker_config
  // TODO
  std::set<export_type> enabled_export_types = {export_type::prometheus, export_type::stdout_type};

  // export format
  // this should be set as well
  // TODO
  export_format fmt = export_format::plain_text;

  // enable container tracing
  // we can get container id and container name
  // using pid from the map of it
  bool enable_container_manager = false;

  // tracing config
  tracing_type tracing_selected = tracing_type::all;
  // tracing targets
  std::string container_name = "";
  unsigned long target_contaienr_id = 0;
  pid_t target_pid = 0;

  bool is_auto_exit = false;
  int exit_after = 0;

  // TODO: this should be add to export config
  std::string prometheus_listening_address = "127.0.0.1:8528";

  std::vector<rule_config> rules;
  std::vector<std::string> seccomp;

};

int trans_string2enum(const std::vector<std::string> &strs, std::string to_trans);
int trans_string2enum(const std::vector<std::string> &strs, std::string_view to_trans);

void analyze_toml(std::string file_path, config &config_toml);
#endif

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
#include "toml.hpp"

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
  };

  // export config
  // may be we should have config similar to tracker_config
  // TODO
  std::set<export_type> enabled_export_types = {export_type::prometheus, export_type::stdout_type};

  // export format
  // this should be set as well
  // TODO
  export_format fmt = export_format::json_format;

  // tracing config
  tracing_type tracing_selected = tracing_type::all;

  // enable container tracing
  // we can get container id and container name
  // using pid from the map of it
  bool enable_container_manager = false;

  // tracing targets
  std::string container_name = "";
  unsigned long target_contaienr_id = 0;
  pid_t target_pid = 0;

  bool is_auto_exit = false;
  int exit_after = 0;

  // TODO: this should be add to export config
  std::string prometheus_listening_address = "127.0.0.1:8528";
};


using namespace std::string_view_literals;

struct database_data
{
    std::string endpoint;
    unsigned int port;
    std::string database;
    std::string username;
    std::string pwd;
};

struct trackers_config
{
    std::string tracker_name;
    unsigned long container_id;
    unsigned int process_id;
    unsigned int run_time;
    std::string fmt;
};

struct rule_config 
{
    std::string rule_name;
    std::string type;
    std::string trigger;
    std::string err_msg;
};

struct exporter_config
{
    std::string exporter_name;
    std::string endpoint;
    unsigned int port;
};

struct toml_config
{
    database_data db;
    std::vector<trackers_config> trackers;
    std::vector<rule_config> rules;
    std::vector<std::string> seccomp;
    std::vector<exporter_config> exporters;
};

static void analyze_toml(std::string file_path, toml_config &config_toml) {
    toml::table data;
    unsigned int i, len;
    try
    {
        data = toml::parse_file(file_path);
    } 
    catch(const std::exception& e)
    {
        std::cerr << e.what() << '\n';
    }
    
    /* fill db */
    config_toml.db.endpoint = std::string(data["MySQL"]["Endpoint"].value_or(""sv));
    config_toml.db.port = data["MySQL"]["Port"].value_or(0);
    config_toml.db.username = std::string(data["MySQL"]["Username"].value_or(""sv));
    config_toml.db.pwd = std::string(data["MySQL"]["Password"].value_or(""sv));
    config_toml.db.database = std::string(data["MySQL"]["Database"].value_or(""sv));
    /* fill trackers */
    len = data["trackers"]["Enable"].as_array()->size();
    for (i = 0; i < len; i++)
    {
        std::string_view tracker_name = data["trackers"]["Enable"][i].value_or(""sv);
        trackers_config tracker_config;
        tracker_config.tracker_name = std::string(tracker_name);
        tracker_config.container_id = data[tracker_name]["container_id"].value_or(0);
        tracker_config.process_id = data[tracker_name]["process_id"].value_or(0);
        tracker_config.run_time = data[tracker_name]["run_time"].value_or(0);
        tracker_config.fmt = data[tracker_name]["fmt"].value_or(""sv);
        config_toml.trackers.emplace_back(tracker_config);
    }

    /* fill rules */
    len = data["rules"]["enable"].as_array()->size();
    for (i = 0; i < len; i++)
    {
        std::string_view rule_name = data["rules"]["Enable"][i].value_or(""sv);
        rule_config rule;
        rule.rule_name = data[rule_name]["name"].value_or(""sv);
        rule.type = data[rule_name]["type"].value_or(""sv);
        rule.err_msg = data[rule_name]["error_message"].value_or(""sv);
        config_toml.rules.emplace_back(rule);
    }

    /* fill seccomp */
    len = data["seccomp"]["allow"].as_array()->size();
    for (i = 0; i < len; i++)
    {
        config_toml.seccomp.emplace_back(std::string(data["seccomp"]["allow"][i].value_or(""sv)));
    }
    
    /* fill exporter */
    len = data["exporter"]["Enable"].as_array()->size();
    for (i = 0; i < len; i++)
    {
        exporter_config exporter;
        std::string_view exporter_name = data["exporter"]["Enable"][i].value_or(""sv);
        
        exporter.port = data[exporter_name]["port"].value_or(0);
        exporter.endpoint = std::string(data[exporter_name]["endpoint"].value_or(""sv));
        exporter.exporter_name = std::string(exporter_name);
        config_toml.exporters.emplace_back(std::string(data["seccomp"]["allow"][i].value_or(""sv)));
    }
    

}

#endif
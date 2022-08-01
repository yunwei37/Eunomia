/* SPDX-License-Identifier: GPL-2.0 OR BSD-3-Clause
 *
 * Copyright (c) 2022, 郑昱笙，濮雯旭，张典典（牛校牛子队）
 * All rights reserved.
 */

#include <clipp.h>

#include <string>
#include <vector>

#include "eunomia/eunomia_core.h"

using namespace std::chrono_literals;

enum class eunomia_mode
{
  run,
  safe,
  seccomp,
  server,
  help
};

void run_mode_operation(const std::string& name, eunomia_config_data& core_config)
{
  core_config.enabled_trackers.clear();
  core_config.enabled_trackers.push_back(tracker_config_data{ .name = name });
  eunomia_core core(core_config);
  core.start_eunomia();
}

void safe_mode_opertiaon(eunomia_config_data core_config)
{
  core_config.fmt = "none";
  core_config.enable_sec_rule_detect = true;
  eunomia_core core(core_config);
  core.start_eunomia();
}

void server_mode_operation(bool load_from_config_file, eunomia_config_data core_config)
{
  if (!load_from_config_file)
  {
    core_config.fmt = "none";
    core_config.enable_sec_rule_detect = true;
  }
  std::cout << "start server mode...\n";
  eunomia_core core(core_config);
  core.start_eunomia();
}

void seccomp_mode_operation(eunomia_config_data core_config)
{
  // enable seccomp with config white list
  // if (core_config.seccomp.len >= 439)
  // {
  //   spdlog::error("seccomp config file error : allow syscall cannot bigger
  //   than 439"); exit(0);
  // } else {
  //   enable_seccomp_white_list(core_config.seccomp);
  // }
}

int main(int argc, char* argv[])
{
  bool prometheus_flag = true, container_flag = false, safe_flag = true;
  ;
  bool load_from_config_file = false;
  pid_t target_pid = 0;
  int time_tracing = 0;
  std::string fmt = "", listening_address = "127.0.0.1:8528";
  std::string config_file = "", output_file = "", container_log_path = "";
  eunomia_mode selected = eunomia_mode::help;
  std::string run_selected = "process";
  unsigned long container_id = 0;

  // spdlog::set_level(spdlog::level::debug);

  auto container_id_cmd = (clipp::option("-c", "--container") & clipp::value("container id", container_id)) %
                          "The conatienr id of the contaienr the EUNOMIA will monitor";
  auto process_id_cmd = (clipp::option("-p", "--process") & clipp::value("process id", target_pid)) %
                        "The process id of the process the EUNOMIA will monitor";
  auto run_time_cmd =
      (clipp::option("-T") & clipp::value("trace time in seconds", time_tracing)) % "The time the ENUNOMIA will monitor for";

  auto run_required_cmd = clipp::value("run required cmd name", run_selected);

  auto config_cmd =
      (clipp::option("--config") & clipp::value("config file", config_file)) % "The toml file stores the config data";

  auto run_mode =
      (clipp::command("run").set(selected, eunomia_mode::run),
       run_required_cmd,
       container_id_cmd,
       process_id_cmd,
       run_time_cmd,
       config_cmd,
       (clipp::option("-m").set(container_flag, true) & clipp::opt_value("path to store dir", container_log_path)) %
           "Start container manager to trace contaienr.",
       (clipp::option("--fmt") & clipp::value("output format of the program", fmt)) %
           "The output format of EUNOMIA, it could be \"json\", \"csv\", "
           "\"plain_txt\", and \"plain_txt\" is the default "
           "choice.");

  auto safe_mode = (clipp::command("safe").set(selected, eunomia_mode::safe), config_cmd);

  auto seccomp_mode =
      (clipp::command("seccomp").set(selected, eunomia_mode::seccomp),
       process_id_cmd,
       run_time_cmd,
       config_cmd,
       (clipp::option("-o") & clipp::opt_value("output file name", output_file)) % "The output file name of seccomp");

  auto server_cmd =
      (clipp::command("server").set(selected, eunomia_mode::server),
       config_cmd,
       (clipp::option("--no_safe").set(safe_flag, false)) % "Stop safe module",
       (clipp::option("--no_prometheus").set(prometheus_flag, false)) % "Stop prometheus server",
       (clipp::option("--listen") & clipp::value("listening address", listening_address)) %
           "Listen http requests on this address, the format is like "
           "\"127.0.0.1:8528\"");

  auto cli = ((run_mode | safe_mode | seccomp_mode | server_cmd | clipp::command("help").set(selected, eunomia_mode::help)));

  if (!clipp::parse(argc, argv, cli))
  {
    std::cout << clipp::make_man_page(cli, argv[0]);
    return 1;
  }

  eunomia_config_data core_config;
  if (config_file != "")
  {
    core_config = eunomia_config_data::from_toml_file(config_file);
    load_from_config_file = true;
  }
  // set base on flags
  // note: cmd flags will override config file
  if (prometheus_flag)
  {
    core_config.prometheus_listening_address = listening_address;
  }
  if (container_id)
  {
    core_config.tracing_target_id = container_id;
  }
  if (target_pid)
  {
    core_config.tracing_target_id = target_pid;
  }
  if (time_tracing > 0)
  {
    core_config.exit_after = time_tracing;
    core_config.is_auto_exit = true;
  }

  spdlog::info("eunomia run in cmd...");

  switch (selected)
  {
    case eunomia_mode::run: run_mode_operation(run_selected, core_config); break;
    case eunomia_mode::safe:
      core_config.enable_sec_rule_detect = true;
      safe_mode_opertiaon(core_config);
      break;
    case eunomia_mode::server: server_mode_operation(load_from_config_file, core_config); break;
    case eunomia_mode::seccomp: seccomp_mode_operation(core_config); break;
    case eunomia_mode::help:
    gdefault:
      std::cout << clipp::make_man_page(cli, argv[0]);
      break;
  }
  return 0;
}

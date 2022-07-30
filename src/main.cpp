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

void run_mode_operation(avaliable_tracker selected, config core_config)
{
  core_config.enabled_trackers.clear();
  switch (selected)
  {
    case avaliable_tracker::tcp:
      core_config.enabled_trackers.push_back(std::make_shared<tcp_tracker_data>(avaliable_tracker::tcp));
      break;
    case avaliable_tracker::syscall:
      core_config.enabled_trackers.push_back(std::make_shared<syscall_tracker_data>(avaliable_tracker::syscall));
      break;
    case avaliable_tracker::process:
      core_config.enabled_trackers.push_back(std::make_shared<process_tracker_data>(avaliable_tracker::process));
      break;
    case avaliable_tracker::files:
      core_config.enabled_trackers.push_back(std::make_shared<files_tracker_data>(avaliable_tracker::files));
      break;
    case avaliable_tracker::ipc:
      core_config.enabled_trackers.push_back(std::make_shared<ipc_tracker_data>(avaliable_tracker::ipc));
      break;
    default: break;
  }
  eunomia_core core(core_config);
  core.start_eunomia();
}

void safe_mode_opertiaon(config core_config)
{
  core_config.fmt = export_format::none;
  core_config.enable_sec_rule_detect = true;
  eunomia_core core(core_config);
  core.start_eunomia();
}

void server_mode_operation(bool load_from_config_file, config core_config)
{
  // std::cout << prometheus_flag << " " << listening_address << " " <<
  // std::endl;
  if (!load_from_config_file)
  {
    core_config.fmt = export_format::none;
    core_config.enable_sec_rule_detect = true;
  }
  std::cout << "start server mode...\n";
  eunomia_core core(core_config);
  core.start_eunomia();
  /*
    TODO
  */
}

void seccomp_mode_operation(config core_config)
{
  /*
    TODO
  */
  spdlog::warn("seccomp mode is not ready yet");
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
  avaliable_tracker run_selected = avaliable_tracker::help;
  unsigned long container_id = 0;

  // spdlog::set_level(spdlog::level::debug);

  auto container_id_cmd = (clipp::option("-c", "--container") & clipp::value("container id", container_id)) %
                          "The conatienr id of the contaienr the EUNOMIA will monitor";
  auto process_id_cmd = (clipp::option("-p", "--process") & clipp::value("process id", target_pid)) %
                        "The process id of the process the EUNOMIA will monitor";
  auto run_time_cmd =
      (clipp::option("-T") & clipp::value("trace time in seconds", time_tracing)) % "The time the ENUNOMIA will monitor for";

  auto run_required_cmd =
      (clipp::option("tcpconnect").set(run_selected, avaliable_tracker::tcp) |
       clipp::option("syscall").set(run_selected, avaliable_tracker::syscall) |
       clipp::option("ipc").set(run_selected, avaliable_tracker::ipc) |
       clipp::option("process").set(run_selected, avaliable_tracker::process) |
       clipp::option("files").set(run_selected, avaliable_tracker::files));

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

  if (!parse(argc, argv, cli))
  {
    std::cout << clipp::make_man_page(cli, argv[0]);
    return 1;
  }

  config core_config = { .enable_container_manager = container_flag, .container_log_path = container_log_path };
  if (config_file != "")
  {
    core_config.enabled_trackers.clear();
    analyze_toml(config_file, core_config);
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
    core_config.target_contaienr_id = container_id;
  }
  if (target_pid)
  {
    core_config.target_pid = target_pid;
  }
  if (time_tracing > 0)
  {
    core_config.exit_after = time_tracing;
    core_config.is_auto_exit = true;
  }
  if (!load_from_config_file && fmt != "")
  {
    int idx = trans_string2enum(str_export_format, fmt);
    if (idx >= 0)
    {
      core_config.fmt = export_format(idx);
    }
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

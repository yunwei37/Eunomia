#include <clipp.h>

#include <string>
#include <vector>

#include "eunomia/eunomia_core.h"

// run mode config
enum class run_cmd_mode
{
  tcpconnect,
  syscall,
  ipc,
  files,
  process,
  help
};

using namespace std::chrono_literals;

void run_mode_operation(
    run_cmd_mode selected,
    config core_config)
{
  core_config.enabled_trackers.clear();
  switch (selected)
  {
    case run_cmd_mode::tcpconnect: break;
    case run_cmd_mode::syscall: break;
    case run_cmd_mode::process:
      core_config.enabled_trackers.push_back(std::make_shared<process_tracker_data>(avaliable_tracker::process));
      break;
    case run_cmd_mode::files:
      core_config.enabled_trackers.push_back(std::make_shared<files_tracker_data>(avaliable_tracker::files));
      break;
    default: break;
  }
  eunomia_core core(core_config);
  core.start_eunomia();
  /*
    TODO
  */
}

void daemon_mode_opertiaon(
    config core_config)
{
  // std::cout << contaienr_id << " " << target_pid << " " << config_file << " " << syscall_id_file << " \n";
  /*
    TODO
  */
}

void server_mode_operation(
    bool prometheus_flag,
    std::vector<std::string>& input,
    config core_config)
{
  // std::cout << prometheus_flag << " " << listening_address << " " << std::endl;
  std::cout << "start server mode...\n";
  eunomia_core core(core_config);
  core.start_eunomia();
  /*
    TODO
  */
}

void seccomp_mode_operation(config core_config)
{
  // std::cout << target_pid << " " << time_tracing << " " << output_file << "\n";
  /*
    TODO
  */
}

int main(int argc, char* argv[])
{
  bool process_flag = false, syscall_flag = false, container_flag = false, ipc_flag = false, tcp_flag = false,
       prometheus_flag = false, file_flag = false;
  pid_t target_pid = 0;
  int time_tracing = 0;
  std::string remote_url = "", fmt = "json", output_file = "";

  eunomia_mode selected = eunomia_mode::help;
  run_cmd_mode run_selected = run_cmd_mode::help;
  unsigned long container_id = 0;
  std::string listening_address = "127.0.0.1:8528";
  std::string config_file = "../test.toml", module_name = "", syscall_id_file = "";
  unsigned int listening_port = 0;
  std::vector<std::string> input;

  auto container_id_cmd =
      (clipp::option("-c", "--container").set(container_flag, true) & clipp::value("container id", container_id)) %
      "The conatienr id of the contaienr the EUNOMIA will monitor";
  auto process_id_cmd = (clipp::option("-p", "--process").set(process_flag, true) & clipp::value("process id", target_pid)) %
                        "The process id of the process the EUNOMIA will monitor";
  auto run_time_cmd =
      (clipp::option("-T") & clipp::value("trace time in seconds", time_tracing)) % "The time the ENUNOMIA will monitor for";

  auto run_required_cmd =
      (clipp::option("tcpconnect").set(run_selected, run_cmd_mode::tcpconnect) |
       clipp::option("syscall").set(run_selected, run_cmd_mode::syscall) |
       clipp::option("ipc").set(run_selected, run_cmd_mode::ipc) |
       clipp::option("process").set(run_selected, run_cmd_mode::process) |
       clipp::option("files").set(run_selected, run_cmd_mode::files));
  auto config_cmd =
      (clipp::option("--config") & clipp::value("config file", config_file)) % "The toml file stores the config data";

  auto run_mode =
      (clipp::command("run").set(selected, eunomia_mode::run),
       run_required_cmd,
       container_id_cmd,
       process_id_cmd,
       run_time_cmd,
       config_cmd,
       (clipp::option("--fmt") & clipp::value("output format of the program", fmt)) % "The output format of EUNOMIA");

  auto daemon_mode =
      (clipp::command("daemon").set(selected, eunomia_mode::daemon),
       container_id_cmd,
       process_id_cmd,
       config_cmd,
       (clipp::option("--seccomp") & clipp::opt_value("syscall id file", syscall_id_file)) % "The syscall table");

  auto seccomp_mode =
      (clipp::command("seccomp").set(selected, eunomia_mode::seccomp),
       process_id_cmd,
       run_time_cmd,
       config_cmd,
       (clipp::option("-o") & clipp::opt_value("output file name", output_file)) % "The output file name of seccomp");

  auto server_cmd =
      (clipp::command("server").set(selected, eunomia_mode::server),
       config_cmd,
       (clipp::option("--prometheus").set(prometheus_flag, true)) % "Start prometheus server",
       (clipp::option("--listen") & clipp::value("listening address", listening_address)) %
           "Listen http requests on this address",
       (clipp::option("--config") & clipp::values("config files", input)) % "Config files input");

  auto cli =
      ((run_mode | daemon_mode | seccomp_mode | server_cmd | clipp::command("help").set(selected, eunomia_mode::help)));

  if (!parse(argc, argv, cli))
  {
    std::cout << clipp::make_man_page(cli, argv[0]);
    return 1;
  }

  config core_config {
    .run_selected = selected,
    .target_contaienr_id = container_id,
    .target_pid = target_pid,
    .prometheus_listening_address = listening_address,
  };
  if (time_tracing > 0)
  {
    core_config.exit_after = time_tracing;
    core_config.is_auto_exit = true;
  }
  if (config_file != "") {
    analyze_toml(config_file, core_config);
  }

  tracker_manager manager;
  container_manager container_manager;
  std::cout << "start ebpf...\n";

  switch (selected)
  {
    case eunomia_mode::run:
      run_mode_operation(run_selected, core_config);
      break;
    case eunomia_mode::daemon:
      daemon_mode_opertiaon(core_config);
      break;
    case eunomia_mode::server: server_mode_operation(prometheus_flag, input, core_config); break;
    case eunomia_mode::seccomp: seccomp_mode_operation(core_config); break;
    case eunomia_mode::help:
    default: std::cout << clipp::make_man_page(cli, argv[0]); break;
  }

  return 0;
}

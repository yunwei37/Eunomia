#include <clipp.h>
#include <vector>
#include <string>

#include "eunomia/container.h"
#include "eunomia/prometheus_server.h"
#include "eunomia/tracker_manager.h"
#include "eunomia/config.h"

// run mode config
enum class run_cmd_mode {tcpconnect, syscall, ipc, file, help};

using namespace std::chrono_literals;

void run_mode_operation(run_cmd_mode selected, unsigned long contaienr_id, pid_t target_pid, int time_tracing, std::string fmt) {
  // int tmp = selected;
  std::cout<<static_cast<std::underlying_type<run_cmd_mode>::type>(selected)<<std::endl;
  std::cout<<" "<<contaienr_id<<" "<<target_pid<<" "<<time_tracing<<" "<<fmt<<std::endl;

  tracker_manager manager;
  container_manager container_manager;
  std::cout << "start ebpf tracker...\n";
  switch (selected)
  {
  case run_cmd_mode::tcpconnect:
  
    
    break;
  
  default:
    break;
  }
  /* 
    TODE 
  */
}
void daemon_mode_opertiaon(unsigned long contaienr_id, pid_t target_pid, std::string config_file, std::string syscall_id_file) {
  std::cout<<contaienr_id<<" "<<target_pid<<" "<<config_file<<" "<<syscall_id_file<<" \n";
  /* 
    TODE 
  */
}
void server_mode_operation(bool prometheus_flag, std::string listening_address, std::vector<std::string> &input) {
  std::cout<<prometheus_flag<<" "<<listening_address<<" "<<input[0]<<std::endl;

  /* 
    TODE 
  */
}
void seccomp_mode_operation(pid_t target_pid, int time_tracing, std::string output_file) {
  std::cout<<target_pid<<" "<<time_tracing<<" "<<output_file<<"\n";
  /* 
    TODE 
  */
}

int main(int argc, char* argv[])
{
  bool process_flag = false, syscall_flag = false, container_flag = false, 
      ipc_flag = false, tcp_flag = false, prometheus_flag = false, file_flag = false;
  pid_t target_pid = 0;
  int time_tracing = 0;
  std::string remote_url = "", fmt = "json", output_file = "";
  
  cmd_mode selected = cmd_mode::help;
  run_cmd_mode run_selected = run_cmd_mode::help;
  unsigned long container_id = 0;
  std::string config_file = "", module_name = "", syscall_id_file = "";
  std::string listening_address = "127.0.0.1:8528";
  std::vector<std::string> input;

  auto container_id_cmd = (clipp::option("-c","--container").set(container_flag, true) & clipp::value("container id", container_id)) % "The conatienr id of the contaienr the EUNOMIA will monitor";
  auto process_id_cmd = (clipp::option("-p", "--process").set(process_flag, true) & clipp::value("process id", target_pid)) % "The process id of the process the EUNOMIA will monitor";
  auto run_time_cmd = (clipp::option("-T") & clipp::value("trace time in seconds", time_tracing)) % "The time the ENUNOMIA will monitor for";
  auto run_required_cmd = (
    clipp::option("tcpconnect").set(run_selected, run_cmd_mode::tcpconnect) |
    clipp::option("syscall").set(run_selected, run_cmd_mode::syscall) |
    clipp::option("ipc").set(run_selected, run_cmd_mode::ipc) |
    clipp::option("file").set(run_selected, run_cmd_mode::file)
  );

  auto run_mode = (
    clipp::command("run").set(selected,cmd_mode::run),
    run_required_cmd,
    container_id_cmd,
    process_id_cmd,
    run_time_cmd,
    (clipp::option("--fmt") & clipp::value("output format of the program", fmt)) % "The output format of EUNOMIA"
  );
  
  auto daemon_mode = (
    clipp::command("daemon").set(selected,cmd_mode::daemon),
    container_id_cmd,
    process_id_cmd,
    (clipp::option("--config") & clipp::value("config file", config_file)) % "The toml file stores the config data",
    (clipp::option("--seccomp") & clipp::opt_value("syscall id file", syscall_id_file)) % "The syscall table"
  );
  
  auto seccomp_mode = (
    clipp::command("seccomp").set(selected, cmd_mode::seccomp),
    process_id_cmd,
    run_time_cmd,
    (clipp::option("-o") & clipp::opt_value("output file name", output_file)) % "The output file name of seccomp"
  );

  auto server_cmd = (
    clipp::command("server").set(selected, cmd_mode::server),
    (clipp::option("--prometheus").set(prometheus_flag, true)) % "Start prometheus server",
    (clipp::option("--listen") & clipp::value("listening address", listening_address)) % "Listen http requests on this address",
    (clipp::option("--config") & clipp::values("config files", input)) % "Config files input"
  );
  


  auto cli = (
    (run_mode | daemon_mode | seccomp_mode | server_cmd | clipp::command("help").set(selected, cmd_mode::help))
  );

  if (!parse(argc, argv, cli))
  {
    std::cout << clipp::make_man_page(cli, argv[0]);
    return 1;
  }

  switch (selected)
  {
  case cmd_mode::run:
    run_mode_operation(run_selected, container_id, target_pid, time_tracing, fmt);
    break;
  case cmd_mode::daemon:
    daemon_mode_opertiaon(container_id, target_pid, config_file, syscall_id_file);
    break;
  case cmd_mode::server:
    server_mode_operation(prometheus_flag, listening_address, input);
    break;
  case cmd_mode::seccomp:
    seccomp_mode_operation(target_pid, time_tracing, output_file);
    break;
  case cmd_mode::help:
  default:
    std::cout << clipp::make_man_page(cli, argv[0]);
    break;
  }
  
  return 0;
}

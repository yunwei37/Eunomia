#include <clipp.h>
#include <vector>
#include <string>

#include "eunomia/container.h"
#include "eunomia/prometheus_server.h"
#include "eunomia/tracker_manager.h"

using namespace std::chrono_literals;

enum class cmd_mode {run, daemon, seccomp, server, help};
enum class run_cmd_mode {tcpconnect, syscall, ipc, file, help};

void run_mode_operation(run_cmd_mode selected, unsigned long contaienr_id, pid_t target_pid, int time_tracing, std::string fmt) {
  // int tmp = selected;
  std::cout<<static_cast<std::underlying_type<run_cmd_mode>::type>(selected)<<std::endl;
  std::cout<<" "<<contaienr_id<<" "<<target_pid<<" "<<time_tracing<<" "<<fmt<<std::endl;
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
void server_mode_operation(bool prometheus_flag, unsigned int listening_port, std::vector<std::string> &input) {
  std::cout<<prometheus_flag<<" "<<listening_port<<" "<<input[0]<<std::endl;

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
  unsigned int listening_port = 0;
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
    (clipp::option("--listen") & clipp::value("listening port", listening_port)) % "Listen http requests on this port",
    (clipp::option("--config") & clipp::values("config files", input)) % "Config files input"
  );
  


  auto cli = (
    (run_mode | daemon_mode | seccomp_mode | server_cmd | clipp::command("help").set(selected, cmd_mode::help))
      //   clipp::option("-p", "--process").set(process_flag).doc("run process ebpf program"),
      //  clipp::option("-s", "--syscall").set(syscall_flag).doc("run syscall ebpf program"),
      //  clipp::option("-c", "--container").set(container_flag).doc("run container ebpf program"),
      //  clipp::option("-t", "--tcp").set(tcp_flag).doc("run tcp ebpf program"),
      //  clipp::option("-i", "--ipc").set(ipc_flag).doc("run ipc ebpf program"),
      //  clipp::option("-u", "--url") & clipp::value("remote url", remote_url),
      //  clipp::option("-P") & clipp::value("trace pid", target_pid),
      //  clipp::option("-T") & clipp::value("trace time in seconds", time_tracing),
      //  clipp::option("-v").set(verbose).doc("print verbose output")
  );

  if (!parse(argc, argv, cli))
  {
    std::cout << clipp::make_man_page(cli, argv[0]);
    return 1;
  }

  tracker_manager manager;
  container_manager container_manager;
  std::cout << "start ebpf...\n";

  switch (selected)
  {
  case cmd_mode::run:
    run_mode_operation(run_selected, container_id, target_pid, time_tracing, fmt);
    break;
  case cmd_mode::daemon:
    daemon_mode_opertiaon(container_id, target_pid, config_file, syscall_id_file);
    break;
  case cmd_mode::server:
    server_mode_operation(prometheus_flag, listening_port, input);
    break;
  case cmd_mode::seccomp:
    seccomp_mode_operation(target_pid, time_tracing, output_file);
    break;
  case cmd_mode::help:
    
    break;
  }
  /*
  int i1 = manager.start_process_tracker();
  int i2 = manager.start_syscall_tracker();
  sleep(3);
  manager.remove_tracker(i1);
  sleep(3);
  manager.remove_tracker(i2);
  i1 = manager.start_process_tracker();
  i2 = manager.start_syscall_tracker();
  sleep(3);
  return 0;
  */
/*
  auto server = prometheus_server("127.0.0.1:8528");

  if (process_flag)
  {
    auto tracker_ptr = std::make_unique<process_tracker>(process_env{}, server);
    manager.start_tracker(std::move(tracker_ptr));
  }
  if (syscall_flag)
  {
    auto tracker_ptr = std::make_unique<syscall_tracker>(syscall_env{});
    manager.start_tracker(std::move(tracker_ptr));
  }
  if (container_flag)
  {
    container_manager.start_container_tracing();
  }
  if (tcp_flag)
  {
  }
  if (ipc_flag)
  {
  }
  if (time_tracing)
  {
    // stop after time_tracing seconds
    std::this_thread::sleep_for(time_tracing * 1s);
    return 0;
  }
  while (1)
    ;
    */
  return 0;
}

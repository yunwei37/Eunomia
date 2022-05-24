#include <clipp.h>

#include "eunomia/prometheus_server.h"
#include "eunomia/tracker_manager.h"
#include "eunomia/container.h"

using namespace std::chrono_literals;

int main(int argc, char* argv[])
{
  bool process_flag = false, syscall_flag = false, container_flag = false, 
      ipc_flag = false, tcp_flag = false, prometheus_flag = false;
  pid_t target_pid = 0;
  int time_tracing = 0;
  std::string remote_url = "", fmt = "json";
  
  enum class cmd_mode {run, daemon, seccomp, server, help};
  cmd_mode selected = cmd_mode::help;
  unsigned long container_id;
  std::string config_file, module_name, syscall_id_file;
  unsigned int listening_port;

  auto container_id_cmd = (clipp::option("-c","--container") & clipp::value("container id", container_id)) % "conatienr id the daemon will run";
  auto process_id_cmd = (clipp::option("-p", "--process") & clipp::value("process id", target_pid)) % "the process id while the daemon will run on";
  auto run_time_cmd = (clipp::option("-T") & clipp::value("trace time in seconds", time_tracing)) % " ";

  auto tcp_run_cmd = (
    clipp::command("tcpconnect").set(tcp_flag, true),
    container_id_cmd,
    process_id_cmd,
    run_time_cmd
  );

  auto run_mode = (
    clipp::command("run").set(selected,cmd_mode::run)
  );
  
  auto daemon_mode = (
    clipp::command("daemon").set(selected,cmd_mode::daemon),
    container_id_cmd,
    process_id_cmd,
    (clipp::option("--config") & clipp::value("config file", config_file)) % " ",
    (clipp::option("--seccomp") & clipp::value("syscall id file", syscall_id_file)) % " "
  );
  
  auto seccomp_mode = (
    clipp::command("seccomp").set(selected, cmd_mode::seccomp),
    process_id_cmd,
    run_time_cmd,
    (clipp::option("-o") & clipp::value("trace time in seconds", time_tracing) )
  );

  auto server_cmd = (
    clipp::command("server").set(selected, cmd_mode::server),
    (clipp::option("--prometheus").set(prometheus_flag, true)) % "start prometheus server",
    (clipp::option("--listen") & clipp::value("listening port", listening_port)) % "start prometheus server"
  );
  


  auto cli = (
    (run_mode | daemon_mode | seccomp_mode | server_cmd | clipp::command("help").set(selected, cmd_mode::help))
      //   clipp::option("-p", "--process").set(process_flag).doc("run process ebpf program"),
      //  clipp::option("-s", "--syscall").set(syscall_flag).doc("run syscall ebpf program"),
      //  clipp::option("-c", "--container").set(container_flag).doc("run container ebpf program"),
      //  clipp::option("-t", "--tcp").set(tcp_flag).doc("run tcp ebpf program"),
      //  clipp::option("-i", "--ipc").set(ipc_flag).doc("run ipc ebpf program"),
      //  clipp::option("-u", "--url") & clipp::value("remote url", remote_url),
      //  clipp::option("-o", "--ouput") & clipp::value("output format", fmt),
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
    /* code */
    break;
  case cmd_mode::daemon:
    break;
  case cmd_mode::server:
    break;
  case cmd_mode::seccomp:
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
  return 0;
}

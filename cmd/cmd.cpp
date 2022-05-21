#include "tracker_manager.h"
#include <clipp.h>

using namespace std::chrono_literals;

bool verbose = false;

int main(int argc, char *argv[]) {
  bool process_flag = false, syscall_flag = false, container_flag = false,
       ipc_flag = false, tcp_flag = false;
  pid_t target_pid = 0;
  int time_tracing = 0;
  std::string remote_url = "", fmt = "json";

  auto cli =
      (clipp::option("-p", "--process")
           .set(process_flag)
           .doc("run process ebpf program"),
       clipp::option("-s", "--syscall")
           .set(syscall_flag)
           .doc("run syscall ebpf program"),
       clipp::option("-c", "--container")
           .set(container_flag)
           .doc("run container ebpf program"),
       clipp::option("-t", "--tcp").set(tcp_flag).doc("run tcp ebpf program"),
       clipp::option("-i", "--ipc").set(ipc_flag).doc("run ipc ebpf program"),
       clipp::option("-u") & clipp::value("remote url", remote_url),
       clipp::option("-o") & clipp::value("output format", fmt),
       clipp::option("-P") & clipp::value("trace pid", target_pid),
       clipp::option("-T") &
           clipp::value("trace time in seconds", time_tracing),
       clipp::option("-v").set(verbose).doc("print verbose output"));

  if (!parse(argc, argv, cli)) {
    std::cout << make_man_page(cli, argv[0]);
    return 1;
  }

  tracker_manager manager;
  std::cout << "start ebpf...\n";

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

  if (process_flag) {
    manager.start_process_tracker();
  }
  if (syscall_flag) {
    manager.start_syscall_tracker();
  }
  if (container_flag) {
  }
  if (tcp_flag) {
  }
  if (ipc_flag) {
  }
  if (time_tracing) {
    // stop after time_tracing seconds
    std::this_thread::sleep_for(time_tracing * 1s);
    return 0;
  }
  while (1)
    ;
  return 0;
}

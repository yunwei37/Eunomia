#include "tracker_manager.h"
#include <clipp.h>

using namespace std::chrono_literals;

bool verbose = false;

int main(int argc, char *argv[]) {
  bool process_flag = false, syscall_flag = false, container_flag = false,
       ipc_flag = false;
  pid_t target_pid = 0;
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
       clipp::option("-i", "--ipc").set(ipc_flag).doc("run ipc ebpf program"),
       clipp::option("-u") & clipp::value("remote url", remote_url),
       clipp::option("-o") & clipp::value("output format", fmt),
       clipp::option("-P") & clipp::value("trace pid", target_pid),
       clipp::option("-v").set(verbose).doc("print verbose output"));

  if (!parse(argc, argv, cli)) {
    std::cout << make_man_page(cli, argv[0]);
    return 1;
  }

  tracker_manager manager;
  std::cout << "start ebpf...\n";

  if (process_flag) {
    manager.start_process_tracker();
    manager.start_process_tracker();
  }
  if (syscall_flag) {
  }
  if (container_flag) {
  }
  if (ipc_flag) {
  }
  while (1)
    ;
  return 0;
}

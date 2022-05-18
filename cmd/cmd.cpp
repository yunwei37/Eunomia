#include <chrono>
#include <clipp.h>
#include <condition_variable>
#include <httplib.h>
#include <iostream>
#include <mutex>
#include <thread>

#include "container.h"
#include "ipc.h"
#include "process.h"
#include "syscall.h"

using namespace std::chrono_literals;

bool verbose = false;

int main(int argc, char *argv[]) {
  bool process_flag = false, syscall_flag = false, container_flag = false,
       ipc_flag = false;
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
       clipp::option("-v").set(verbose).doc("print verbose output"));

  if (!parse(argc, argv, cli)) {
    std::cout << make_man_page(cli, argv[0]);
    return 1;
  }

  process_tracker process_tracker;
  syscall_tracker syscall_tracker;
  container_tracker container_tracker;
  ipc_tracker ipc_tracker;
  std::vector<std::thread> threads;
  std::cout << "start ebpf...\n";

  if (process_flag) {
    threads.emplace_back(&process_tracker::start_process, &process_tracker);
  }
  if (syscall_flag) {
    threads.emplace_back(&syscall_tracker::start_syscall, &syscall_tracker);
  }
  if (container_flag) {
    threads.emplace_back(&container_tracker::start_container,
                         &container_tracker);
  }
  if (ipc_flag) {
    threads.emplace_back(&ipc_tracker::start_ipc, &ipc_tracker);
  }
  for (auto &i : threads) {
    i.join();
  }
  return 0;
}

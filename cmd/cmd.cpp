#include <chrono>
#include <condition_variable>
#include <iostream>
#include <mutex>
#include <thread>

#include "process.h"
#include "syscall.h"

using namespace std::chrono_literals;

#include <clipp.h>

bool verbose = false;

int main(int argc, char *argv[]) {
  bool process_flag = false, syscall_flag = false;
  std::string remote_url = "", fmt = "json";

  auto cli = (clipp::option("-p", "--process")
                  .set(process_flag)
                  .doc("run process ebpf program"),
              clipp::option("-s", "--syscall")
                  .set(syscall_flag)
                  .doc("run syscall ebpf program"),
              clipp::option("-u") & clipp::value("remote url", remote_url),
              clipp::option("-o") & clipp::value("output format", fmt),
              clipp::option("-v").set(verbose).doc("print verbose output"));

  if (!parse(argc, argv, cli)) {
    std::cout << make_man_page(cli, argv[0]);
    return 1;
  }

  process_tracker process_tracker;
  syscall_tracker syscall_tracker;
  std::vector<std::thread> threads;
  std::cout << "start ebpf...\n";

  if (process_flag) {
    threads.emplace_back(&process_tracker::start_process, &process_tracker);
  }
  if (syscall_flag) {
    threads.emplace_back(&syscall_tracker::start_syscall, &syscall_tracker);
  }
  for (auto &i: threads) {
	i.join();
  }
  return 0;
}

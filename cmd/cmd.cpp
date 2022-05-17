#include <chrono>
#include <condition_variable>
#include <iostream>
#include <mutex>
#include <thread>

#include "process.h"
#include "syscall.h"

using namespace std::chrono_literals;

bool verbose = false;

int main() {
  process_tracker process_tracker;
  syscall_tracker syscall_tracker;
  std::thread process_t1(
      &process_tracker::start_process,
      &process_tracker);  // spawn new thread that calls foo()
  // std::thread process_t2(&syscall_tracker::start_syscall, &syscall_tracker);

  std::cout << "main, foo and bar now execute concurrently...\n";
  // std::this_thread::sleep_for(5000ms);
  // process_t.~thread();
  //  synchronize threads:
  process_t1.join();  // pauses until first finishes
  // process_t2.join();

  std::cout << "foo and bar completed.\n";

  return 0;
}

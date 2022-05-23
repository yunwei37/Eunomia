#include <gtest/gtest.h>

#include "eunomia/tracker_manager.h"

using namespace std::chrono_literals;

int main(int argc, char **argv)
{ 
  tracker_manager manager;
  std::cout << "start ebpf...\n";

 auto server = prometheus_server("127.0.0.1:8528");

  auto tracker_ptr = std::make_unique<process_tracker>(process_env{
      .min_duration_ms = 100,
  }, server);
    manager.start_process_tracker(std::move(tracker_ptr));

  server.start_prometheus_server();

  std::this_thread::sleep_for(1000s);
  return 0;
}

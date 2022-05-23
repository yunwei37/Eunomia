#include <gtest/gtest.h>

#include "eunomia/tracker_manager.h"

using namespace std::chrono_literals;

int main(int argc, char **argv)
{ 
  tracker_manager manager;
  std::cout << "start ebpf...\n";

 auto server = prometheus_server("127.0.0.1:8528");
  //server.start_prometheus_server();
  auto& events_counter =
      prometheus::BuildCounter().Name("observed_events_total").Help("Number of observed packets").Register(*server.registry);

  auto tracker_ptr = std::make_unique<process_tracker>(process_env{}, events_counter);
    manager.start_process_tracker(std::move(tracker_ptr));

  std::this_thread::sleep_for(10s);
  return 0;
}

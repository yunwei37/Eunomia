#include "eunomia/files.h"
#include "eunomia/process.h"
#include "eunomia/tracker_manager.h"

using namespace std::chrono_literals;

int main(int argc, char **argv)
{
  tracker_manager manager;

  auto server = prometheus_server("127.0.0.1:8528");
  std::cout << "start ebpf...\n";

  auto prometheus_process_handler =
      std::make_shared<process_tracker::prometheus_event_handler>(process_tracker::prometheus_event_handler(server));
  auto prometheus_files_handler =
      std::make_shared<files_tracker::prometheus_event_handler>(files_tracker::prometheus_event_handler(server));
  // auto json_event_printer = std::make_shared<process_tracker::json_event_printer>(process_tracker::json_event_printer{});
  // auto json_event_printer2 = std::make_shared<process_tracker::json_event_printer>(process_tracker::json_event_printer{});
  // prometheus_event_handler->add_handler(json_event_printer);
  // prometheus_event_handler->add_handler(json_event_printer)->add_handler(json_event_printer2);

  auto process_ptr = process_tracker::create_tracker_with_default_env(prometheus_process_handler);
  auto files_ptr = files_tracker::create_tracker_with_default_env(prometheus_files_handler);
  
  std::cout<< "start tracker..." << std::endl;
  manager.start_tracker(std::move(process_ptr));
  std::cout<< "start tracker..." << std::endl;
  manager.start_tracker(std::move(files_ptr));

  std::cout<< "start server..." << std::endl;
  server.start_prometheus_server();

  std::this_thread::sleep_for(1000s);
  return 0;
}

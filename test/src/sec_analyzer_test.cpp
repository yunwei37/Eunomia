/* SPDX-License-Identifier: GPL-2.0 OR BSD-3-Clause
 *
 * Copyright (c) 2022, 郑昱笙，濮雯旭，张典典（牛校牛子队）
 * All rights reserved.
 */

#include "eunomia/sec_analyzer.h"
#include "eunomia/tracker_manager.h"

using namespace std::chrono_literals;

int main(int argc, char **argv)
{
  tracker_manager manager;
  std::cout << "start ebpf...\n";

  auto server = prometheus_server("127.0.0.1:8528");
  auto sec_analyzer = sec_analyzer::create_sec_analyzer_with_default_rules();
  auto syscall_checker = std::make_shared<syscall_rule_checker>(sec_analyzer);

  auto prometheus_event_handler =
      std::make_shared<syscall_tracker::prometheus_event_handler>(syscall_tracker::prometheus_event_handler(server));
  auto json_event_printer = std::make_shared<syscall_tracker::plain_text_event_printer>(syscall_tracker::plain_text_event_printer{});
  prometheus_event_handler->add_handler(syscall_checker);

  auto tracker_ptr = syscall_tracker::create_tracker_with_default_env(prometheus_event_handler);
  manager.start_tracker(std::move(tracker_ptr));

  server.start_prometheus_server();

  std::this_thread::sleep_for(1000s);
  return 0;
}

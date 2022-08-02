/* SPDX-License-Identifier: GPL-2.0 OR BSD-3-Clause
 *
 * Copyright (c) 2022, 郑昱笙，濮雯旭，张典典（牛校牛子队）
 * All rights reserved.
 */
#include <dirent.h>

#include <algorithm>
#include <cerrno>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <regex>
#include <stdexcept>

#include "eunomia/container_manager.h"
#include "eunomia/process.h"
#include "eunomia/tcp.h"
#include "eunomia/tracker_manager.h"

using namespace std::chrono_literals;

int main(int argc, char** argv)
{
  {
    container_manager mp;
    tracker_manager manager;
    std::cout << "start ebpf...\n";

    auto server = prometheus_server("127.0.0.1:8528");

    // auto stdout_event_printer =
    //     std::make_shared<process_tracker::plain_text_event_printer>(process_tracker::plain_text_event_printer{});
    auto container_tracking_handler =
        std::make_shared<container_manager::container_tracking_handler>(container_manager::container_tracking_handler{ mp });

    // container_tracking_handler->add_handler(stdout_event_printer);
    auto stdout_event_printer =
         std::make_shared<tcp_tracker::plain_text_event_printer>(tcp_tracker::plain_text_event_printer{});
    auto container_info_handler =
        std::make_shared<container_manager::container_info_handler<tcp_event>>(container_manager::container_info_handler<tcp_event>{ mp });
    container_info_handler->add_handler(stdout_event_printer);

    manager.start_tracker(process_tracker::create_tracker_with_default_env(container_tracking_handler));
    manager.start_tracker(tcp_tracker::create_tracker_with_default_env(container_info_handler));

    server.start_prometheus_server();

    std::this_thread::sleep_for(10s);
  }
  return 0;
}

/* SPDX-License-Identifier: GPL-2.0 OR BSD-3-Clause
 *
 * Copyright (c) 2022, 郑昱笙，濮雯旭，张典典（牛校牛子队）
 * All rights reserved.
 */

#include <httplib.h>

#include <json.hpp>

#include "eunomia/container_manager.h"
extern "C"
{
#include <process/process.h>
}

using namespace nlohmann;
using namespace std::chrono_literals;

class container_client
  {
   private:
    // for dockerd
    httplib::Client dockerd_client;

   public:
    // get all container info json string
    std::string list_all_containers(void) {
      std::stringstream ss;
      auto response = dockerd_client.Get("/containers/json");
      ss << response->body;
      return ss.str();
    }
    // get container process by id
    std::string list_all_process_running_in_container(const std::string &container_id) {
  std::stringstream ss;
  auto response = dockerd_client.Get("/containers/" + container_id + "/top");
  ss << response->body;
  return ss.str();
}

    // get container info by id
    std::string inspect_container(const std::string &container_id);
    container_info get_os_container_info(void);
    container_client() : dockerd_client("unix:/var/run/docker.sock") {
      dockerd_client.set_default_headers({ { "Host", "localhost" } });
    }
  };

int main(int argc, char** argv)
{
  container_manager mp;
  return 0;
}

/* SPDX-License-Identifier: GPL-2.0 OR BSD-3-Clause
 *
 * Copyright (c) 2022, 郑昱笙，濮雯旭，张典典（牛校牛子队）
 * All rights reserved.
 */

#include <spdlog/spdlog.h>
#include <string.h>

#include <sstream>

#include "eunomia/container_manager.h"
#include "httplib.h"
#include "json.hpp"

extern "C"
{
#include <container/container.h>
#include <process/process_tracker.h>
#include <unistd.h>
}

using namespace nlohmann;

container_manager::container_client::container_client() : dockerd_client("unix:/var/run/docker.sock")
{
  dockerd_client.set_default_headers({ { "Host", "localhost" } });
}

std::string container_manager::container_client::list_all_containers(void)
{
  std::stringstream ss;
  auto response = dockerd_client.Get("/containers/json");
  ss << response->body;
  return ss.str();
}

std::string container_manager::container_client::list_all_process_running_in_container(const std::string& container_id)
{
  std::stringstream ss;
  auto response = dockerd_client.Get("/containers/" + container_id + "/top");
  ss << response->body;
  return ss.str();
}

std::string container_manager::container_client::inspect_container(const std::string& container_id)
{
  std::stringstream ss;
  auto response = dockerd_client.Get("/containers/" + container_id + "/json");
  ss << response->body;
  return ss.str();
}

container_info container_manager::container_client::get_os_container_info(void)
{
  try
  {
    json res_json;
    std::stringstream ss;
    auto response = dockerd_client.Get("/info");
    ss << response->body;
    ss >> res_json;
    return container_info{
      .id = "0",
      .name = res_json["OperatingSystem"].get<std::string>(),
    };
  }
  catch (...)
  {
    spdlog::error("Failed to get os container info, is dockerd running?");
    return container_info{
      .id = "0",
      .name = "Bare metal",
    };
  }
}

container_manager::container_manager()
{
  os_info = client.get_os_container_info();
  spdlog::info("OS container info: {}", os_info.name);
  init_container_map_data();
}

void container_manager::init_container_map_data(void)
{
  auto response = client.list_all_containers();
  json containers_json = json::parse(response);
  for (const auto c : containers_json)
  {
    container_info info = { c["Id"], c["Names"][0], container_status_from_str(c["State"]) };

    json process_resp = json::parse(client.list_all_process_running_in_container(info.id));
    for (const auto p : process_resp["Processes"])
    {
      info_map.insert(std::atoi(std::string(p[2]).c_str()), info);
    }
  }
}
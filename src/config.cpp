/* SPDX-License-Identifier: GPL-2.0 OR BSD-3-Clause
 *
 * Copyright (c) 2022, 郑昱笙，濮雯旭，张典典（牛校牛子队）
 * All rights reserved.
 */

#include "eunomia/config.h"

#include <spdlog/spdlog.h>

#include <json.hpp>

#include "toml.hpp"

#define get_from_json_at(name)     \
  try                              \
  {                                \
    j.at(#name).get_to(data.name); \
  }                                \
  catch (...)                      \
  {                                \
  }

static void from_json(const nlohmann::json& j, handler_config_data& data)
{
  get_from_json_at(name);
  get_from_json_at(args);
}

static void from_json(const nlohmann::json& j, tracker_config_data& data)
{
  get_from_json_at(name);
  get_from_json_at(args);
  get_from_json_at(export_handlers);
}

static void from_json(const nlohmann::json& j, rule_config_data& data)
{
  get_from_json_at(rule_name);
  get_from_json_at(type);
  get_from_json_at(err_msg);
  get_from_json_at(trigger);
}

static void from_json(const nlohmann::json& j, seccomp_config_data& data)
{
  get_from_json_at(allow_syscall);
  get_from_json_at(container_id);
}

static void from_json(const nlohmann::json& j, eunomia_config_data& data)
{
  try
  {
    get_from_json_at(run_selected);
    get_from_json_at(enabled_trackers);
    get_from_json_at(tracing_selected);
    get_from_json_at(tracing_target_id);
    get_from_json_at(is_auto_exit);
    get_from_json_at(exit_after);
    get_from_json_at(enabled_export_types);
    get_from_json_at(fmt);
    get_from_json_at(enable_container_manager);
    get_from_json_at(prometheus_listening_address);
    get_from_json_at(enable_sec_rule_detect);
    get_from_json_at(security_rules);
    get_from_json_at(seccomp_data);
    get_from_json_at(enable_seccomp_module);
  }
  catch (...)
  {
    spdlog::info("some part of toml is not right!");
    exit(0);
  };
}

eunomia_config_data eunomia_config_data::from_toml_file(const std::string& file_path)
{
  toml::table data;
  try
  {
    data = toml::parse_file(file_path);
  }
  catch (const toml::parse_error& err)
  {
    spdlog::error("parse toml file error: {}", err);
    return eunomia_config_data{};
  }
  auto json_data = toml::json_formatter{ data };
  std::stringstream ss;
  ss << json_data;
  nlohmann::json j = ss.str();
  return j.get<eunomia_config_data>();
}

eunomia_config_data eunomia_config_data::from_json_file(const std::string& file_path)
{
  std::ifstream i(file_path);
  nlohmann::json j;
  i >> j;
  return j.get<eunomia_config_data>();
}

tracker_config_data tracker_config_data::from_json_str(const std::string& json_str)
{
  try
  {
    nlohmann::json j = nlohmann::json::parse(json_str);
    return j.get<tracker_config_data>();
  }
  catch (...)
  {
    spdlog::error("json parse error for tracker_config_data! {}", json_str);
  }
  return tracker_config_data{};
}

rule_config_data rule_config_data::from_json_str(const std::string& json_str)
{
  try
  {
    nlohmann::json j = nlohmann::json::parse(json_str);
    return j.get<rule_config_data>();
  }
  catch (...)
  {
    spdlog::error("json parse error for rule_config_data! {}", json_str);
  }
  return rule_config_data{};
}

/* SPDX-License-Identifier: GPL-2.0 OR BSD-3-Clause
 *
 * Copyright (c) 2022, 郑昱笙，濮雯旭，张典典（牛校牛子队）
 * All rights reserved.
 */

#include "eunomia/config.h"

#include <spdlog/spdlog.h>

#include "toml.hpp"

using namespace std::string_view_literals;

int trans_string2enum(const std::vector<std::string>& strs, std::string to_trans)
{
  unsigned int i, len = strs.size();
  for (i = 0; i < len; i++)
  {
    if (strs[i] == to_trans)
    {
      break;
    }
  }
  if (i == len)
  {
    return -1;
  }
  return i;
}

int trans_string2enum(const std::vector<std::string>& strs, std::string_view to_trans)
{
  std::size_t i, len = strs.size();
  for (i = 0; i < len; i++)
  {
    if (strs[i] == to_trans)
    {
      break;
    }
  }
  if (i == len)
  {
    return -1;
  }
  return (int)i;
}

void analyze_toml(std::string file_path, config& config_toml)
{
  toml::table data;
  std::size_t i, len;
  try
  {
    data = toml::parse_file(file_path);
  }
  catch (const toml::parse_error& err)
  {
    std::cerr << err << "\n";
    return;
  }
  /* fill trackers part */
  len = data["trackers"]["Enable"].as_array()->size();
  for (i = 0; i < len; i++)
  {
    std::string_view tracker_name = data["trackers"]["Enable"][i].value_or(""sv);
    if (tracker_name == "syscall")
    {
      config_toml.enabled_trackers.emplace_back(std::make_shared<syscall_tracker_data>(avaliable_tracker::syscall));
    }
    else if (tracker_name == "process")
    {
      config_toml.enabled_trackers.emplace_back(std::make_shared<process_tracker_data>(avaliable_tracker::process));
    }
    else if (tracker_name == "ipc")
    {
      config_toml.enabled_trackers.emplace_back(std::make_shared<ipc_tracker_data>(avaliable_tracker::ipc));
    }
    else if (tracker_name == "tcp")
    {
      config_toml.enabled_trackers.emplace_back(std::make_shared<tcp_tracker_data>(avaliable_tracker::tcp));
    }
    else if (tracker_name == "files")
    {
      config_toml.enabled_trackers.emplace_back(std::make_shared<files_tracker_data>(avaliable_tracker::files));
    }
    else
    {
      spdlog::error("Wrong format of trackers in toml!");
      exit(0);
    }
  }
  config_toml.target_contaienr_id = data["trackers"]["container_id"].value_or(0);
  config_toml.target_pid = data["trackers"]["process_id"].value_or(0);
  config_toml.exit_after = data["trackers"]["run_time"].value_or(0);
  if (config_toml.exit_after)
  {
    config_toml.is_auto_exit = true;
  }

  /* check whether the fmt is legal */
  int idx = trans_string2enum(str_export_format, data["trackers"]["fmt"].value_or(""sv));
  if (idx >= 0)
  {
    config_toml.fmt = export_format(idx);
  }
  else
  {
    spdlog::error("Wrong input of fmt!");
    exit(0);
  }

  // /* fill rules */
  len = data["rules"]["Enable"].as_array()->size();
  for (i = 0; i < len; i++)
  {
    std::string_view rule_name = data["rules"]["Enable"][i].value_or(""sv);
    rule_config rule;
    rule.rule_name = data[rule_name]["name"].value_or(""sv);
    rule.type = data[rule_name]["type"].value_or(""sv);
    rule.err_msg = data[rule_name]["error_message"].value_or(""sv);
    config_toml.rules.emplace_back(rule);
  }

  /* fill seccomp */
  len = data["seccomp"]["allow"].as_array()->size();
  for (i = 0; i < len; i++)
  {
    config_toml.seccomp.emplace_back(std::string(data["seccomp"]["allow"][i].value_or(""sv)));
  }

  /* fill exporter */
  len = data["exporter"]["Enable"].as_array()->size();
  for (i = 0; i < len; i++)
  {
    std::string_view exporter_name = data["exporter"]["Enable"][i].value_or(""sv);
    idx = trans_string2enum(str_export_type, exporter_name);
    if (idx < 0)
    {
      spdlog::error("The format of toml is not right in exporter!");
      exit(0);
    }
    config_toml.enabled_export_types.insert(export_type(idx));
  }

  /* fill container part */
  config_toml.enable_container_manager = data["container"]["Enable"].value_or(false);
  if (config_toml.enable_container_manager)
  {
    config_toml.container_log_path = data["container"]["log_path"].value_or(""sv);
  }
}

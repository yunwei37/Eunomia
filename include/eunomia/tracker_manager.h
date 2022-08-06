/* SPDX-License-Identifier: GPL-2.0 OR BSD-3-Clause
 *
 * Copyright (c) 2022, 郑昱笙，濮雯旭，张典典（牛校牛子队）
 * All rights reserved.
 */

#ifndef TRACKER_MANAGER_H
#define TRACKER_MANAGER_H

#include <condition_variable>
#include <iostream>
#include <mutex>
#include <thread>
#include <map>
#include "model/tracker.h"

struct tracker_manager
{
 private:
  struct tracker_base_data {
    std::string name;
    std::unique_ptr<tracker_base> tracker;
  };
  int id_count = 1;
  std::map<int,  tracker_base_data> trackers;

 public:
  ~tracker_manager() = default;
  void remove_tracker(int id)
  {
    trackers.erase(id);
  }
  std::vector<std::tuple<int, std::string>> get_tracker_list()
  {
    std::vector<std::tuple<int, std::string>> list;
    for (auto &[id, data] : trackers) {
      list.push_back({id, data.name});
    }
    return list;
  }
  std::size_t start_tracker(std::unique_ptr<tracker_base> tracker_ptr, const std::string &name)
  {
    if (!tracker_ptr)
    {
      std::cout << "tracker_ptr is null in start_tracker\n";
      return 0;
    }
    tracker_ptr->thread = std::thread(&tracker_base::start_tracker, tracker_ptr.get());
    trackers.emplace(id_count++, tracker_base_data{name, std::move(tracker_ptr)});
    return trackers.size() - 1;
  }

  void remove_all_trackers()
  {
    trackers.clear();
  }
};

#endif

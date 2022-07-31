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
  int id_count = 1;
  std::map<int, std::unique_ptr<tracker_base>> trackers;

 public:
  ~tracker_manager() = default;
  void remove_tracker(int id)
  {
    trackers.erase(id);
  }

  std::size_t start_tracker(std::unique_ptr<tracker_base> tracker_ptr)
  {
    if (!tracker_ptr)
    {
      std::cout << "tracker_ptr is null in start_tracker\n";
      return 0;
    }
    tracker_ptr->thread = std::thread(&tracker_base::start_tracker, tracker_ptr.get());
    trackers.emplace(id_count++, std::move(tracker_ptr));
    return trackers.size() - 1;
  }

  void remove_all_trackers()
  {
    trackers.clear();
  }
};

#endif

#ifndef TRACKER_MANAGER_H
#define TRACKER_MANAGER_H

#include <condition_variable>
#include <iostream>
#include <mutex>
#include <thread>
#include "model/tracker.h"

struct tracker_manager
{
 private:
  int id_count = 0;
  std::map<int, std::unique_ptr<tracker_base>> trackers;

 public:
  void remove_tracker(int id)
  {
    trackers.erase(id);
  }

  std::size_t start_tracker(std::unique_ptr<tracker_base> tracker_ptr)
  {
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

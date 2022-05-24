#ifndef TRACKER_MANAGER_H
#define TRACKER_MANAGER_H

#include <condition_variable>
#include <iostream>
#include <mutex>
#include <thread>

#include "ipc.h"
#include "process.h"
#include "syscall.h"
#include "tcp.h"

struct tracker_manager {
private:
  int id_count = 0;
  std::map<int, std::unique_ptr<tracker>> trackers;

public:
  void remove_tracker(int id) { trackers.erase(id); }

  std::size_t start_tracker(std::unique_ptr<tracker> tracker_ptr) {
    tracker_ptr->thread =
        std::thread(&tracker::start_tracker, tracker_ptr.get());
    trackers.emplace(id_count++, std::move(tracker_ptr));
    return trackers.size() - 1;
  }

  void remove_all_trackers() {
    trackers.clear();
  }
};

#endif

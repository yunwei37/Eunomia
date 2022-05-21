#ifndef TRACKER_MANAGER_H
#define TRACKER_MANAGER_H

#include <condition_variable>
#include <httplib.h>
#include <iostream>
#include <mutex>
#include <thread>

#include "container.h"
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
  int start_process_tracker(process_env env) {
    auto tracker_ptr = std::make_unique<process_tracker>(env);
    tracker_ptr->thread =
        std::thread(&process_tracker::start_tracker, tracker_ptr.get());
    trackers.emplace(id_count++, std::move(tracker_ptr));
    return trackers.size() - 1;
  }
  int start_process_tracker() { return start_process_tracker({}); }

  int start_syscall_tracker(syscall_env env) {
    auto tracker_ptr = std::make_unique<syscall_tracker>(env);
    tracker_ptr->thread =
        std::thread(&syscall_tracker::start_tracker, tracker_ptr.get());
    trackers.emplace(id_count++, std::move(tracker_ptr));
    return trackers.size() - 1;
  }
  int start_syscall_tracker() { return start_syscall_tracker({}); }
};

#endif

#ifndef TRAKER_H
#define TRAKER_H

#include <iostream>
#include <mutex>
#include <thread>

#include "tracker_config.h"
struct tracker {
  std::thread thread;
  volatile bool exiting;
  std::mutex mutex;

public:
  virtual ~tracker() {
    exiting = true;
    if (thread.joinable()) {
      thread.join();
    }
  }
  virtual void start_tracker(void) = 0;
  void stop_tracker(void) { exiting = true; }
};

template <typename ENV, typename EVENT>
struct tracker_with_config : public tracker {
  tracker_config<ENV, EVENT> config;
  tracker_with_config(tracker_config<ENV, EVENT> config) : config(config) {}
};

#endif
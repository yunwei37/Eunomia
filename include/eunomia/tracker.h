#ifndef TRAKER_H
#define TRAKER_H

#include <iostream>
#include <mutex>
#include <thread>

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

#endif
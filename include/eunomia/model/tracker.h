#ifndef TRAKER_H
#define TRAKER_H

#include <iostream>
#include <mutex>
#include <thread>

#include "tracker_config.h"

// the base type of a tracker
// for tracker manager to manage
struct tracker_base
{
  std::thread thread;
  volatile bool exiting;
  std::mutex mutex;

 public:
  virtual ~tracker_base()
  {
    exiting = true;
    if (thread.joinable())
    {
      thread.join();
    }
  }
  virtual void start_tracker(void) = 0;
  void stop_tracker(void)
  {
    exiting = true;
  }
};

// all tracker should inherit from this class
template<typename ENV, typename EVENT>
struct tracker_with_config : public tracker_base
{
  tracker_config<ENV, EVENT> current_config;
  tracker_with_config(tracker_config<ENV, EVENT> config) : current_config(config)
  {
  }
};

template<typename TRACKER>
concept C = requires
{
  typename TRACKER::current_config;  // required nested member name
};

// function for handler tracker event call back
// used when running a tracker
// Example:
//
// start_process_tracker(handle_tracker_event<process_tracker, process_event>, libbpf_print_fn, current_config.env, skel,
// (void *)this);
template<typename TRACKER, typename EVENT>
static int handle_tracker_event(void *ctx, void *data, size_t data_sz)
{
  if (!data || !ctx)
  {
    return -1;
  }
  const EVENT &e = *(const EVENT *)data;
  TRACKER &pt = *(TRACKER *)ctx;
  auto event = tracker_event<EVENT>{ e };
  if (pt.current_config.handler)
  {
    pt.current_config.handler->do_handle_event(event);
  }
  return 0;
}

#endif

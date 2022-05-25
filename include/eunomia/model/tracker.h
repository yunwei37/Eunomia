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
  // base thread
  std::thread thread;
  volatile bool exiting;
  // TODO: use the mutex
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
concept tracker_concept = requires
{
  typename TRACKER::config_data;
  typename TRACKER::tracker_event_handler;
  typename TRACKER::prometheus_event_handler;
  typename TRACKER::json_event_printer;
};

// function for handler tracker event call back
// used when running a tracker
// Example:
//
// start_process_tracker(handle_tracker_event<process_tracker, process_event>, libbpf_print_fn, current_config.env, skel,
// (void *)this);
template<tracker_concept TRACKER, typename EVENT>
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
  } else {
    std::cout<< "warn: no handler for tracker event" << std::endl;
  }
  return 0;
}

#endif

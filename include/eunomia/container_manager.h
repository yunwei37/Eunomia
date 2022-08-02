#ifndef CONTAINER_MANAGER_EUNOMIA_H
#define CONTAINER_MANAGER_EUNOMIA_H

#include <mutex>
#include <shared_mutex>
#include <unordered_map>
#include <httplib.h>
#include "model/tracker.h"

extern "C"
{
#include <process/process.h>
}

class container_manager
{
 public:
  // use process tracker to track the processes created in the container
  class container_tracking_handler : public event_handler<process_event>
  {
   public:
    void handle(tracker_event<process_event> &e);
    container_tracking_handler(container_manager &manager);
  };

  template<typename EVENT>
  // use process tracker to track the processes created in the container
  class container_info_handler : public event_handler<EVENT>
  {
   public:
    void handle(tracker_event<EVENT> &e);
    container_info_handler(container_manager &manager);
  };

  container_manager();

 private:
  // container client for getting container info
  class container_client
  {
   private:
    // for dockerd
    httplib::Client dockerd_client;

   public:
    // get all container info json string
    std::string list_all_containers(void);
    // get container process by id
    std::string list_all_process_running_in_container(const std::string &container_id);
    // get container info by id
    std::string inspect_container(const std::string &container_id);
    container_info get_os_container_info(void);
    container_client();
  };

  // used to store container info
  class container_info_map
  {
   private:
    // use rw lock to protect the map
    mutable std::shared_mutex mutex_;
    std::unordered_map<int, container_info> container_info_map__;

   public:
    container_info_map() = default;
    // insert a container info into the map
    void insert(int pid, container_info info)
    {
      std::unique_lock<std::shared_mutex> lock(mutex_);
      container_info_map__[pid] = info;
    }
    // get a container info from the map
    container_info get(int pid)
    {
      std::shared_lock<std::shared_mutex> lock(mutex_);
      return container_info_map__[pid];
    }
    // remove a pid related container info from the map
    void remove(int pid)
    {
      std::unique_lock<std::shared_mutex> lock(mutex_);
      container_info_map__.erase(pid);
    }
  };

  container_info_map info_map;
  container_client client;

  // This is the default info for process not in the container
  container_info os_info;

  void init_container_map_data(void);
};

#endif
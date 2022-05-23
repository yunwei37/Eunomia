#ifndef CONTAINER_CMD_H
#define CONTAINER_CMD_H

#include <iostream>
#include <json.hpp>
#include <unordered_map>
#include <vector>
#include <stdio.h>
#include <thread>
#include <mutex>

#include "libbpf_print.h"
#include "tracker.h"

extern "C" {
// #include <container/container_tracker.h>
#include <process/process_tracker.h>
#include <unistd.h>
}

using json = nlohmann::json;
static std::unordered_map<int, struct process_event> container_processes;
pthread_rwlock_t rwlock;

struct container_tracker : public tracker {
  struct process_env env = {0};
  container_tracker(process_env env) : env(env) {
    // do container settings
    env.exclude_current_ppid = ::getpid();
    env.min_duration_ms = 20;
    exiting = false;
    this->env.exiting = &exiting;
  }
  void start_tracker() {
    struct process_bpf *skel;
    start_process_tracker(handle_event, libbpf_print_fn, env, skel);
  }
  static std::string to_json(const struct process_event &e) {
    std::string res;
    json process_event = {{"type", "process"},
                          {"time", get_current_time()},
                          {"pid", e.common.pid},
                          {"ppid", e.common.ppid},
                          {"cgroup_id", e.common.cgroup_id},
                          {"user_namespace_id", e.common.user_namespace_id},
                          {"pid_namespace_id", e.common.pid_namespace_id},
                          {"mount_namespace_id", e.common.mount_namespace_id},
                          {"exit_code", e.exit_code},
                          {"duration_ns", e.duration_ns},
                          {"comm", e.comm},
                          {"filename", e.filename},
                          {"exit_event", e.exit_event}};
    return process_event.dump();
  }

  static int judge_container(const struct process_event &e) {
    int ret = 0;
    if (e.exit_code)
    {
      pthread_rwlock_wrlock(&rwlock);
      container_processes.erase(e.common.pid);
      pthread_rwlock_unlock(&rwlock);
    } else {
      /* parent process exist in map */
      pthread_rwlock_rdlock(&rwlock);
      auto event = container_processes.find(e.common.ppid);
      pthread_rwlock_unlock(&rwlock);
      if (event != container_processes.end())
      {
        if (container_processes[e.common.ppid].common.cgroup_id != e.common.cgroup_id)
        {
          ret = 1;
        }
      } else {
        unsigned int cgroup;
        std::string cmd("ls -Li /proc/");
        cmd += std::to_string(e.common.ppid);
        cmd += "/ns/cgroup";
        FILE *fp = popen((char *)cmd.c_str(), "r");
        if(fp) {
          if (fscanf(fp, "%u %*[^\n]\n", &cgroup) == 1) {
            if (cgroup != e.common.cgroup_id) {
              ret = 1;
            }
          }
        }
        pclose(fp);
      }
      pthread_rwlock_wrlock(&rwlock);
      container_processes[e.common.pid] = e;
      pthread_rwlock_unlock(&rwlock);
    }

    return ret;
  }
  
  static int handle_event(void *ctx, void *data, size_t data_sz) {
    if (!data) {
      return -1;
    }
    const struct process_event *e = (const struct process_event *)data;
    /* judge whether this a container related cmd */
    if(judge_container(*e)) {
      std::cout << "\033[31m" << to_json(*e) << "\033[0m"<< std::endl;  
    }
    return 0;
  }
};

#endif
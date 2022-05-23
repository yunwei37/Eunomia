#ifndef CONTAINER_CMD_H
#define CONTAINER_CMD_H

#include <iostream>
#include <json.hpp>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <stdio.h>
#include <thread>
#include <mutex>

#include "libbpf_print.h"
#include "tracker.h"

extern "C" {
// #include <container/container_tracker.h>
#include <container/container.h>
#include <process/process_tracker.h>
#include <unistd.h>
}

using json = nlohmann::json;
static std::unordered_map<int, struct container_event> container_processes;
// static std::unordered_set<unsigned long> containers;
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
    printf("%-10s %-15s %-20s %-10s\n", "PID", "PARENT_PID", "CONTAINER_ID", "STATE");
    init_container_table();
    start_process_tracker(handle_event, libbpf_print_fn, env, skel, (void*)this);
  }

  static void fill_event(struct process_event &event) {
    std::string cmd("ls -Li /proc/"), cgroup("/ns/cgroup"), user("/ns/user"),
                pid("/ns/pid"), mnt("/ns/mnt");
    unsigned int ns;
    FILE *fp;

    cmd += std::to_string(event.common.pid);

    fp = popen((cmd + cgroup).c_str(), "r");
    fscanf(fp, "%u %*[^\n]\n", &ns);
    event.common.cgroup_id = ns;
    fclose(fp);

    fp = popen((cmd + user).c_str(), "r");
    fscanf(fp, "%u %*[^\n]\n", &ns);
    event.common.user_namespace_id = ns;
    fclose(fp);

    fp = popen((cmd + mnt).c_str(), "r");
    fscanf(fp, "%u %*[^\n]\n", &ns);
    event.common.mount_namespace_id = ns;
    fclose(fp);

    fp = popen((cmd + pid).c_str(), "r");
    fscanf(fp, "%u %*[^\n]\n", &ns);
    event.common.pid_namespace_id = ns;
    fclose(fp);
  }

  static void init_container_table() {
    unsigned long cid;
    const std::vector<std::string> ns = {"cgroup", "user", "pid", "mnt"};
    pid_t pid, ppid;
    const char *ps_cmd = "docker ps -q";
    FILE *ps = popen(ps_cmd, "r");
    while (fscanf(ps, "%lu\n", &cid) == 1)
    {
      std::string top_cmd("docker top ");
      top_cmd += std::to_string(cid);
      FILE *top = popen(top_cmd.c_str(), "r");
      if (top == NULL) {
      }
      /* delet the first row */
      char useless[150];
      fgets(useless, 150, top);
      while (fscanf(top, "%*s %d %d %*[^\n]\n", &pid, &ppid) == 2)
      {
        struct process_event event;
        event.common.pid = pid;
        event.common.ppid = ppid;
        fill_event(event);
        struct container_event con = {
          .process = event,
          .container_id = cid,
          .has_printed = false,
        };
        print_container(con);
        container_processes[pid] = con;
      }
      
    }
    
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

  static void print_container(const struct container_event &e) {
    std::string state = e.process.exit_event == true ? "EXIT" : "EXEC";
    printf("%-10d %-15d %-20ld %-10s\n", e.process.common.pid, 
                                        e.process.common.ppid, 
                                        e.container_id, 
                                        state.c_str());
    // e.has_printed = true;
  }

  static int judge_container(const struct process_event &e) {
    int ret = 0;
    if (e.exit_code)
    {
      auto event = container_processes.find(e.common.pid);
      if (event != container_processes.end()) {
        event->second.process.exit_event = true;
        print_container(event->second);
        container_processes.erase(event);
      }
    } else {
      /* parent process exist in map */
      pthread_rwlock_rdlock(&rwlock);
      auto event = container_processes.find(e.common.ppid);
      pthread_rwlock_unlock(&rwlock);
      if (event != container_processes.end()) {
        struct container_event con = {
          .process = e,
          .container_id = (*event).second.container_id,
          .has_printed = false,
        };
        pthread_rwlock_wrlock(&rwlock);
        container_processes[e.common.pid] = con;
        print_container(container_processes[e.common.pid]);
        pthread_rwlock_unlock(&rwlock);
      } else {
        if (1/* judge the cgroup and other data*/)
        {
          FILE *fp = popen("docker ps -q", "r");
          unsigned long cid;
          /* show all alive container */
          pid_t pid, ppid;
          while (fscanf(fp, "%lu\n", &cid) == 1)
          {
            // printf("%lu\n", cid);
            std::string top_cmd = "docker top ";
            top_cmd += std::to_string(cid);
            FILE *top = popen(top_cmd.c_str(), "r");
            char useless[150];
            /* delet the first row */
            fgets(useless, 150, top);
            while (fscanf(top, "%*s %d %d %*[^\n]\n", &pid, &ppid) == 2)
            {
              // printf("wish %d %d\n", pid, ppid);
              /* this is the first show time for this process */
              if (container_processes.find(pid) == container_processes.end()) {                  struct container_event con = {0};
                struct container_event cont = {
                  .process = e,
                  .container_id = cid,
                  .has_printed = false,
                };
                container_processes[pid] = cont;
                print_container(container_processes[pid]);
                ret = 1;
              }
            }
          }
        }
    
      }
    }

    return ret;
  }
  
  static int handle_event(void *ctx, void *data, size_t data_sz) {
    if (!data) {
      return -1;
    }
    const struct process_event *e = (const struct process_event *)data;
    /* judge whether this a container related cmd */
    judge_container(*e);
    return 0;
  }
};

#endif
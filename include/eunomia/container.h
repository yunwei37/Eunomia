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
#include <memory>

#include "libbpf_print.h"
#include "tracker.h"

extern "C" {
#include <container/container.h>
#include <process/process_tracker.h>
#include <unistd.h>
}

using json = nlohmann::json;
static std::unordered_map<int, struct container_event> container_processes;
static std::mutex mp_lock;

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
    cmd += std::to_string(event.common.pid);

    std::unique_ptr<FILE, int(*)(FILE*)> fp_cgroup(popen((cmd + cgroup).c_str(), "r"), pclose);
    fscanf(fp_cgroup.get(), "%u %*[^\n]\n", &ns);
    event.common.cgroup_id = ns;

    std::unique_ptr<FILE, int(*)(FILE*)> fp_user(popen((cmd + user).c_str(), "r"), pclose);
    fscanf(fp_user.get(), "%u %*[^\n]\n", &ns);
    event.common.user_namespace_id = ns;
    
    std::unique_ptr<FILE, int(*)(FILE*)> fp_mnt(popen((cmd + mnt).c_str(), "r"), pclose);
    fscanf(fp_mnt.get(), "%u %*[^\n]\n", &ns);
    event.common.mount_namespace_id = ns;

    std::unique_ptr<FILE, int(*)(FILE*)> fp_pid(popen((cmd + pid).c_str(), "r"), pclose);
    fscanf(fp_pid.get(), "%u %*[^\n]\n", &ns);
    event.common.pid_namespace_id = ns;
  }

  static void init_container_table() {
    unsigned long cid;
    pid_t pid, ppid;
    char *ps_cmd = "docker ps -q";
    std::unique_ptr<FILE, int(*)(FILE*)> ps(popen(ps_cmd, "r"), pclose);
    while (fscanf(ps.get(), "%lx\n", &cid) == 1)
    {
      std::string top_cmd("docker top ");
      char hex_cid[20];
      sprintf(hex_cid, "%lx", cid);
      top_cmd += hex_cid;
      std::unique_ptr<FILE, int(*)(FILE*)> top(popen(top_cmd.c_str(), "r"), pclose);
      // FILE *top = popen(top_cmd.c_str(), "r");
      /* delet the first row */
      char useless[150];
      fgets(useless, 150, top.get());
      while (fscanf(top.get(), "%*s %d %d %*[^\n]\n", &pid, &ppid) == 2)
      {
        struct process_event event;
        event.common.pid = pid;
        event.common.ppid = ppid;
        fill_event(event);
        struct container_event con = {
          .process = event,
          .container_id = cid,
        };
        print_container(con);
        mp_lock.lock();
        container_processes[pid] = con;
        mp_lock.unlock();
      }
    }
  }

  static void print_container(const struct container_event &e) {
    std::string state = e.process.exit_event == true ? "EXIT" : "EXEC";
    printf("%-10d %-15d %-20lx %-10s\n", e.process.common.pid, 
                                        e.process.common.ppid, 
                                        e.container_id, 
                                        state.c_str());
  }

  static void judge_container(const struct process_event &e) {
    if (e.exit_event)
    {
      mp_lock.lock();
      auto event = container_processes.find(e.common.pid);
      if (event != container_processes.end()) {
        event->second.process.exit_event = true;
        print_container(event->second);
        container_processes.erase(event);
      }
      mp_lock.unlock();
    } else {
      /* parent process exist in map */
      mp_lock.lock();
      auto event = container_processes.find(e.common.ppid);
      mp_lock.unlock();
      if (event != container_processes.end()) {
        struct container_event con = {
          .process = e,
          .container_id = (*event).second.container_id,
        };
        mp_lock.lock();
        container_processes[e.common.pid] = con;
        print_container(container_processes[e.common.pid]);
        mp_lock.unlock();
      } else {
        if (1/* judge the cgroup and other data*/)
        {
          std::unique_ptr<FILE, int(*)(FILE*)> fp(popen("docker ps -q", "r"), pclose);
          // FILE *fp = ;
          unsigned long cid;
          /* show all alive container */
          pid_t pid, ppid;
          while (fscanf(fp.get(), "%lx\n", &cid) == 1)
          {
            std::string top_cmd = "docker top ";
            char hex_cid[20];
            sprintf(hex_cid, "%lx", cid);
            top_cmd += hex_cid;
            std::unique_ptr<FILE, int(*)(FILE*)> top(popen(top_cmd.c_str(), "r"), pclose);
            char useless[150];
            /* delet the first row */
            fgets(useless, 150, top.get());
            while (fscanf(top.get(), "%*s %d %d %*[^\n]\n", &pid, &ppid) == 2)
            {
              mp_lock.lock();
              /* this is the first show time for this process */
              if (container_processes.find(pid) == container_processes.end()) {                  
                struct container_event con = {
                  .process = e,
                  .container_id = cid,
                };
                container_processes[pid] = con;
                print_container(container_processes[pid]);
              }
              mp_lock.unlock();
            }
          }
        }
    
      }
    }
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
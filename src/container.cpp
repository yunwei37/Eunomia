#include "eunomia/container.h"

extern "C"
{
#include <container/container.h>
#include <process/process_tracker.h>
#include <unistd.h>
}

container_tracker::container_tracker(container_env env, container_manager &manager)
    : tracker_with_config(tracker_config<container_env, container_event>{}),
      current_env(env),
      this_manager(manager)
{
  // do container settings
  // this->env.process_env.exclude_current_ppid = ::getpid();
  // ignore short process
  this->current_env.penv.min_duration_ms = 20;
  exiting = false;
  this->current_env.penv.exiting = &exiting;
}

void container_tracker::start_tracker()
{
  struct process_bpf *skel = nullptr;
  if (current_env.print_result)
    printf("%-10s %-15s %-20s %-25s %-10s\n", "PID", "PARENT_PID", "CONTAINER_ID", "CONTAINER_NAME", "STATE");
  init_container_table();
  start_process_tracker(handle_event, libbpf_print_fn, current_env.penv, skel, (void *)this);
}

void container_tracker::fill_event(struct process_event &event)
{
  std::string cmd("ls -Li /proc/"), cgroup("/ns/cgroup"), user("/ns/user"), pid("/ns/pid"), mnt("/ns/mnt");
  unsigned int ns;
  cmd += std::to_string(event.common.pid);
  std::unique_ptr<FILE, int (*)(FILE *)> fp_cgroup(popen((cmd + cgroup).c_str(), "r"), pclose);
  fscanf(fp_cgroup.get(), "%u %*[^\n]\n", &ns);
  event.common.cgroup_id = ns;
  std::unique_ptr<FILE, int (*)(FILE *)> fp_user(popen((cmd + user).c_str(), "r"), pclose);
  fscanf(fp_user.get(), "%u %*[^\n]\n", &ns);
  event.common.user_namespace_id = ns;
  std::unique_ptr<FILE, int (*)(FILE *)> fp_mnt(popen((cmd + mnt).c_str(), "r"), pclose);
  fscanf(fp_mnt.get(), "%u %*[^\n]\n", &ns);
  event.common.mount_namespace_id = ns;
  std::unique_ptr<FILE, int (*)(FILE *)> fp_pid(popen((cmd + pid).c_str(), "r"), pclose);
  fscanf(fp_pid.get(), "%u %*[^\n]\n", &ns);
  event.common.pid_namespace_id = ns;
}
void container_tracker::init_container_table()
{
  unsigned long cid;
  pid_t pid, ppid;
  std::string ps_cmd("docker ps -q");
  std::unique_ptr<FILE, int (*)(FILE *)> ps(popen(ps_cmd.c_str(), "r"), pclose);
  while (fscanf(ps.get(), "%lx\n", &cid) == 1)
  {
    std::string top_cmd("docker top "), name_cmd("docker inspect -f '{{.Name}}' ");
    char hex_cid[20], container_name[50];
    sprintf(hex_cid, "%lx", cid);
    top_cmd += hex_cid;
    name_cmd += hex_cid;
    std::unique_ptr<FILE, int (*)(FILE *)> top(popen(top_cmd.c_str(), "r"), pclose), name(popen(name_cmd.c_str(), "r"), pclose);
    fscanf(name.get(), "/%s", container_name);
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
        .container_name = container_name,
      };
      print_container(con);
      this_manager.mp_lock.lock();
      this_manager.container_processes[pid] = con;
      this_manager.mp_lock.unlock();
    }
  }
}
void container_tracker::print_container(const struct container_event &e)
{
  if (!current_env.print_result)
  {
    return;
  }
  std::string state = e.process.exit_event == true ? "EXIT" : "EXEC";
  printf("%-10d %-15d %-20lx %-25s %-10s\n", e.process.common.pid, e.process.common.ppid, e.container_id, e.container_name.c_str(), state.c_str());
}

void container_tracker::judge_container(const struct process_event &e)
{
  if (e.exit_event)
  {
    this_manager.mp_lock.lock();
    auto event = this_manager.container_processes.find(e.common.pid);
    // remove from map
    if (event != this_manager.container_processes.end())
    {
      event->second.process.exit_event = true;
      print_container(event->second);
      this_manager.container_processes.erase(event);
    }
    this_manager.mp_lock.unlock();
  }
  else
  {
    /* parent process exists in map */
    this_manager.mp_lock.lock();
    auto event = this_manager.container_processes.find(e.common.ppid);
    this_manager.mp_lock.unlock();
    if (event != this_manager.container_processes.end())
    {
      struct container_event con = {
        .process = e,
        .container_id = (*event).second.container_id,
        .container_name = (*event).second.container_name
      };
      this_manager.mp_lock.lock();
      this_manager.container_processes[e.common.pid] = con;
      print_container(this_manager.container_processes[e.common.pid]);
      this_manager.mp_lock.unlock();
    }
    else
    {
      /* parent process doesn't exist in map */
      struct process_event p_event = {0};
      p_event.common.pid = e.common.ppid;
      fill_event(p_event);
      if ((p_event.common.user_namespace_id != e.common.user_namespace_id)
          || (p_event.common.pid_namespace_id != e.common.pid_namespace_id)
          || (p_event.common.mount_namespace_id != e.common.mount_namespace_id))
      {
        std::unique_ptr<FILE, int (*)(FILE *)> fp(popen("docker ps -q", "r"), pclose);
        unsigned long cid;
        /* show all alive container */
        pid_t pid, ppid;
        while (fscanf(fp.get(), "%lx\n", &cid) == 1)
        {
          std::string top_cmd = "docker top ", name_cmd = "docker inspect -f '{{.Name}}' ";
          char hex_cid[20], container_name[50];
          sprintf(hex_cid, "%lx", cid);
          top_cmd += hex_cid;
          name_cmd += hex_cid;
          std::unique_ptr<FILE, int (*)(FILE *)> top(popen(top_cmd.c_str(), "r"), pclose), name(popen(name_cmd.c_str(), "r"), pclose);
          fscanf(name.get(), "/%s", container_name);
          char useless[150];
          /* delet the first row */
          fgets(useless, 150, top.get());
          while (fscanf(top.get(), "%*s %d %d %*[^\n]\n", &pid, &ppid) == 2)
          {
            this_manager.mp_lock.lock();
            /* this is the first show time for this process */
            if (this_manager.container_processes.find(pid) == this_manager.container_processes.end())
            {
              struct container_event con = {
                .process = e,
                .container_id = cid,
                .container_name = container_name,
              };
              this_manager.container_processes[pid] = con;
              print_container(this_manager.container_processes[pid]);
            }
            this_manager.mp_lock.unlock();
          }
        }
      }
    }
  }
}

int container_tracker::handle_event(void *ctx, void *data, size_t data_sz)
{
  if (!data || !ctx)
  {
    return -1;
  }
  const struct process_event &e = *(const struct process_event *)data;
  container_tracker &pt = *(container_tracker *)ctx;
  /* judge whether this a container related cmd */
  pt.judge_container(e);
  return 0;
}

unsigned long container_manager::get_container_id_via_pid(pid_t pid)
{
  unsigned long ret = 0;
  mp_lock.lock();
  auto event = container_processes.find(pid);
  if (event != container_processes.end())
  {
    ret = event->second.container_id;
  }
  mp_lock.unlock();
  return ret;
}

# 容器追踪模块设计

## 容器信息数据结构
目前我们的容器追踪模块是基于进程追踪模块实现的，其数据结构为：
```c
struct container_event {
	struct process_event process;
	unsigned long container_id;
	char container_name[50];
};
```
容器追踪模块由`container_tracker`实现
```cpp
struct container_tracker : public tracker_with_config<container_env, container_event>
{
  struct container_env current_env = { 0 };
  struct container_manager &this_manager;
  std::shared_ptr<spdlog::logger> container_logger;

  container_tracker(container_env env, container_manager &manager);
  void start_tracker();

  void fill_event(struct process_event &event);

  void init_container_table();

  void print_container(const struct container_event &e);

  void judge_container(const struct process_event &e);

  static int handle_event(void *ctx, void *data, size_t data_sz);
};
```
同时我们添加了一个manager类来控制tracker。
```cpp
struct container_manager
{
 private:
  struct tracker_manager tracker;
  std::mutex mp_lock;
  std::unordered_map<int, struct container_event> container_processes;
  friend struct container_tracker;

 public:
  void start_container_tracing(std::string log_path)
  { 
    tracker.start_tracker(std::make_unique<container_tracker>(container_env{
      .log_path = log_path,
      .print_result = true,
    }, *this));
  }
  unsigned long get_container_id_via_pid(pid_t pid);
};
```

## 容器追踪实现

容器追踪模块的ebpf代码服用了process追踪模块的ebpf代码，因此这里我们只介绍用户态下对数据处理的设计。   
当内核态捕捉到进程的数据返回到用户态时，我们调用`judge_container()`函数，判断该进程是否归属于一个container，其具体实现为：
```cpp
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
      struct container_event con = { .process = e, .container_id = (*event).second.container_id };
      strcpy(con.container_name, (*event).second.container_name);
      this_manager.mp_lock.lock();
      this_manager.container_processes[e.common.pid] = con;
      print_container(this_manager.container_processes[e.common.pid]);
      this_manager.mp_lock.unlock();
    }
    else
    {
      /* parent process doesn't exist in map */
      struct process_event p_event = { 0 };
      p_event.common.pid = e.common.ppid;
      fill_event(p_event);
      if ((p_event.common.user_namespace_id != e.common.user_namespace_id) ||
          (p_event.common.pid_namespace_id != e.common.pid_namespace_id) ||
          (p_event.common.mount_namespace_id != e.common.mount_namespace_id))
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
          std::unique_ptr<FILE, int (*)(FILE *)> top(popen(top_cmd.c_str(), "r"), pclose),
              name(popen(name_cmd.c_str(), "r"), pclose);
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
              };
              strcpy(con.container_name, container_name);
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

```

首先，如果进程处于退出状态，那么该函数会直接判断其数据是否已经存在于`container_processes`这一哈希map中。该哈希map专门用于存储归属于容器的进程的信息。如果已经存在这直接输出并删除，否则跳过。如果进程处于执行状态，我们首先会检查该进程的父进程是否存在于`container_processes`中，如果存在则认为此进程也是容器中的进程，将此进程直接加入并输出即可。如果不存在则检查其namespace信息和其父进程是否一致，如果不一致我们会认为此时可能会有一个新的容器产生。对于`Docker`类容器，我们会直接调用`Docker`给出的命令的进行观测。首先调用`docker ps -q`命令获得现有在运行的所有容器id，之后调用`docker top id`命令获取容器中的进程在宿主机上的进程信息，如果这些信息没有被记录到哈希map中，那么就将他们添加到其中并输出。    
由于这一方式无法捕捉到在本追踪器启动前就已经在运行的容器进程，因此我们会在程序启动伊始，调用一次`init_container_table()`函数，其实现为：
```cpp
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
    std::unique_ptr<FILE, int (*)(FILE *)> top(popen(top_cmd.c_str(), "r"), pclose),
        name(popen(name_cmd.c_str(), "r"), pclose);
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
      };
      strcpy(con.container_name, container_name);
      print_container(con);
      this_manager.mp_lock.lock();
      this_manager.container_processes[pid] = con;
      this_manager.mp_lock.unlock();
    }
  }
}
```
该函数的实现逻辑与`judge_contaienr()`函数类似，但是它会将已经在运行的容器进程存入哈希map中，以方便后续追踪。
# 系统框架与ebpf探针设计

<!-- TOC -->

- [系统框架设计](#系统框架设计)
  - [系统设计](#系统设计)
  - [模块设计](#模块设计)
  - [ebpf 探针设计：以线程方式运行的核心探针（初赛阶段完成）](#ebpf-探针设计以线程方式运行的核心探针初赛阶段完成)
    - [ebpf 探针相关 C 代码设计，以 process 为例：](#ebpf-探针相关-c-代码设计以-process-为例)
    - [C++ 部分探针代码设计](#c-部分探针代码设计)
    - [handler 相关事件处理代码](#handler-相关事件处理代码)

<!-- /TOC -->

## 系统框架设计

### 系统设计

<div  align="center">  
 <img src="./imgs/new_arch.jpg" width = "800" height = "450" alt="eunomia_architecture" align=center />
 <p>系统架构</p>
</div>

关于详细的系统架构设计和模块划分，请参考 [系统设计文档](doc/design_doc)

### 模块设计


- tracker_manager

  负责启动和停止 ebpf 探针，并且和 ebpf 探针通信（每个 tracer 是一个线程）；

  - start tracker
  - stop tracker(remove tracker)

  我们主要有五个ebpf探针:

  - process
  - syscall
  - tcp
  - files
  - ipc  
  除此意外我们还集成了大量来自于BCC等开源工具中的追踪器，包括oomkill, memleak等。

- container_manager

  负责观察 container 的启动和停止，保存每个 container 的相关信息：（cgroup，namespace），同时负责 container id, container name 等 container mata 信息到 pid 的转换（提供查询接口）

- seccomp_manager

  负责对 process 进行 seccomp 限制

- handler/data collector

  负责处理 ebpf 探针上报的事件

- security analyzer

  容器安全检测规则引擎和安全分析模块，通过ebpf采集到的底层相关数据，运用包括AI在内的多种方法进行安全性分析，可以帮助您检测事件流中的可疑行为模式。
  
- prometheus exporter

  将数据导出成Prometheus需要的格式，在Prometheus中保存时序数据，方便后续持久化和可视化功能。

- config loader

  解析 toml

- cmd

  命令行解析模块，将命令行字符串解析成对应的参数选项，对Eunomia进行配置。

- core

  负责装配所需要的 tracker，配置对应的功能用例，并且启动系统。

- server

  http 通信：通过 `graphql` 在远程发起 http 请求并执行监控工具，将产生的数据进行聚合后返回，用户可自定义运行时扩展插件进行在线数据分析。这一个部分还没有完成。

### ebpf 探针设计：以线程方式运行的核心探针（初赛阶段完成）

采用 ebpf 探针的方式，可以获取到安全事件的相关信息，并且可以通过 prometheus 监控指标进行监控和分析。

我们的探针代码分为两个部分，其一是在 `bpftools` 中，是针对相关 ebpf 程序的 libbpf 具体探针接口实现，负责1ebpf 程序的加载、配置、以及相关用户态和内核态通信的代码；另外一部分是在 src 中，针对 ebpf 探针上报的信息进行具体处理的 C++ 类实现，负责根据配置决定ebpf上报的信息将会被如何处理。

#### ebpf 探针相关 C 代码设计，以 process 为例：

process 部分的代码主要负责获取进程的执行和退出时和进程相关的以下的信息：

- pid
- cgroup
- namespace：user pid mount
- ppid
- command
- 可执行文件路径

其中容器相关信息会保存起来并被其他 tracker 用以查询。

ebpf 代码：在 bpftools\process\process.bpf.c 中，这里贴出来的代码经过了一定程度的化简。

```c
static __always_inline void fill_event_basic(pid_t pid, struct task_struct *task, struct process_event *e)
{
	e->common.pid = pid;
	e->common.ppid = BPF_CORE_READ(task, real_parent, tgid);
	e->common.cgroup_id = bpf_get_current_cgroup_id();
	e->common.user_namespace_id = get_current_user_ns_id();
	e->common.pid_namespace_id = get_current_pid_ns_id();
	e->common.mount_namespace_id = get_current_mnt_ns_id();
}


SEC("tp/sched/sched_process_exec")
int handle_exec(struct trace_event_raw_sched_process_exec *ctx)
{
	struct task_struct *task;
	unsigned fname_off;
	struct process_event *e;
	pid_t pid;
	u64 ts;

	/* remember time exec() was executed for this PID */
	pid = bpf_get_current_pid_tgid() >> 32;
	if (target_pid && pid != target_pid)
		return 0;
	ts = bpf_ktime_get_ns();
	bpf_map_update_elem(&exec_start, &pid, &ts, BPF_ANY);

	/* reserve sample from BPF ringbuf */
	e = bpf_ringbuf_reserve(&rb, sizeof(*e), 0);
	if (!e)
		return 0;

	/* fill out the sample with data */
	task = (struct task_struct *)bpf_get_current_task();
	if (exclude_current_ppid) {
		if (exclude_current_ppid == BPF_CORE_READ(task, real_parent, tgid)) {
			return 0;
		}
	}
	fill_event_basic(pid, task, e);

	bpf_get_current_comm(&e->comm, sizeof(e->comm));
	e->exit_event = false;
	fname_off = ctx->__data_loc_filename & 0xFFFF;
	bpf_probe_read_str(e->filename, sizeof(e->filename), (void *)ctx + fname_off);

	/* successfully submit it to user-space for post-processing */
	bpf_ringbuf_submit(e, 0);
	return 0;
}

```

这部分是负责处理进程执行的代码，通过挂载点 `tp/sched/sched_process_exec` 来监测所有的进程执行和退出相关情况。其中包含了对进程的相关信息的获取，以及对进程的相关信息的填充。具体进程相关的信息会被放到这个结构体中，并传递给 C++ 编写的处理程序：

bpftools\process\process.h
```c

struct common_event {
	int pid;
	int ppid;
	uint64_t cgroup_id;
	uint32_t user_namespace_id;
	uint32_t pid_namespace_id;
	uint32_t mount_namespace_id;
};

struct process_event
{
	struct common_event common;

	unsigned exit_code;
	unsigned long long duration_ns;
	char comm[TASK_COMM_LEN];
	char filename[MAX_FILENAME_LEN];
	bool exit_code;
};
```

C++ 部分通过 `start_process_tracker` 函数来加载 process 相关 ebpf 探针，并且注册回调函数。以下是相关签名：


代码在 bpftools\process\process_tracker.h 中：
```c
static int start_process_tracker(
    ring_buffer_sample_fn handle_event,
    libbpf_print_fn_t libbpf_print_fn,
    struct process_env env,
    struct process_bpf *skel,
    void *ctx);
```

每个 ebpf 探针会被当做一个独立的线程运行，这个线程会被放到一个单独的线程池中，这样就可以保证每个 ebpf 探针都是独立的进程：

- 我们可以在同一个二进制程序或者进程中同时运行多个探针，例如可以同时运行 process 和 tcp，通过 process 获取的容器元信息，以 pid 作为主键来查询 tcp 每个连接相关的容器信息。
- 探针可以在 eunomia 运行的任意时刻被启动，也可以在任意时刻被关闭。
- 同一种类型的探针可以被运行多个实例，比如来监测不同的 cgroups 或者不同的进程。

这样设计的目的是，例如如果我们需要进行在线的监控数据获取和分析，可以通过远端的 http 请求，让 eunomia 往内核中注入一个 ebpf 探针，运行 30 秒后停止该探针，然后通过 graphql 请求，通过外界数据库或者内置的算子进行数据聚合，之后返回获取的数据指标。这样的好处是可以不用在任何时候都必须运行某些代价高昂的监控服务（例如 syscall 监控），极大地节省相关服务器资源，避免干扰正常的服务运行。

每个探针有两个重要的数据结构， event 和 env。event 上报给用户态的信息结构体， env是对应的 tracker 的配置：

```cpp
struct process_env
{
  bool verbose;
  pid_t target_pid;
  pid_t exclude_current_ppid;
  long min_duration_ms;
  volatile bool *exiting;
};

```

C++ 部分的代码会在调用 start_process_tracker 之前设置好对应的 env 信息，来控制 ebpf 代码的相关行为。

#### C++ 部分探针代码设计

我们采用类似责任链的设计模式，通过一系列的回调函数和事件处理类来处理 ebpf 上报的内核事件：

- 每个 ebpf 探针都是一个单独的类
- 每个探针类都可以有数量不限的事件处理 handler 类（例如转换成 json 类型，上报给 prometheus，打印输出，保存文件，进行聚合等），它们通过类似链表的方式组织起来，并且可以在运行被动态组装；

以 process 为例，c++部分的探针代码如下：

see: include\eunomia\process.h
```cpp
// ebpf process tracker interface
// the true implementation is in process/process_tracker.h
//
// trace process start and exit
struct process_tracker : public tracker_with_config<process_env, process_event>
{
  using config_data = tracker_config<process_env, process_event>;
  using tracker_event_handler = std::shared_ptr<event_handler<process_event>>;

  process_tracker(config_data config);

  // create a tracker with deafult config
  static std::unique_ptr<process_tracker> create_tracker_with_default_env(tracker_event_handler handler);

  process_tracker(process_env env);
  // start process tracker
  void start_tracker();

  // used for prometheus exporter
  struct prometheus_event_handler : public event_handler<process_event>
  {
    prometheus::Family<prometheus::Counter> &eunomia_process_start_counter;
    prometheus::Family<prometheus::Counter> &eunomia_process_exit_counter;
    void report_prometheus_event(const struct process_event &e);

    prometheus_event_handler(prometheus_server &server);
    void handle(tracker_event<process_event> &e);
  };

  // convert event to json
  struct json_event_handler_base : public event_handler<process_event>
  {
    std::string to_json(const struct process_event &e);
  };

  // used for json exporter, inherits from json_event_handler
  struct json_event_printer : public json_event_handler_base
  {
    void handle(tracker_event<process_event> &e);
  };
  
  // used for print to console
  struct plain_text_event_printer : public event_handler<process_event>
  {
    void handle(tracker_event<process_event> &e);
  };
  
};
```

这部分代码继承自 tracker_base，每个 ebpf 探针的代码都会继承自 tracker_base 和 tracker_with_config:

include\eunomia\model\tracker.h
```cpp

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
```

分成两个类设计的目的是为了同时完成运行时多态编译期多态。其中 tracker_config 是对应的模板类，包含了探针的配置信息和处理事件的 handler，比如：

include\eunomia\model\tracker_config.h
```cpp 

// config data for tracker
// pass this to create a tracker
template <typename ENV, typename EVENT>
struct tracker_config
{   
    // tracker env in C code
    ENV env;
    std::string name;
    // event handler interface
    std::shared_ptr<event_handler<EVENT>> handler = nullptr;
};

```

每个 ebpf 探针类都要满足对应的 concept，比如：

include\eunomia\model\tracker.h
```cpp
// concept for tracker
// all tracker should have these types
template<typename TRACKER>
concept tracker_concept = requires
{
  typename TRACKER::config_data;
  typename TRACKER::tracker_event_handler;
  typename TRACKER::prometheus_event_handler;
  typename TRACKER::json_event_printer;
  typename TRACKER::plain_text_event_printer;
};

```

这个 concept 规定了 tracker 必须要实现的 handler ，以及需要有的子类型。

#### handler 相关事件处理代码

每个探针类都可以有数量不限的事件处理 handler 类（例如转换成 json 类型，上报给 prometheus，打印输出，保存文件，进行聚合等），它们通过类似链表的方式组织起来，并且可以在运行被动态组装；

- ebpf 上报的 event 会按顺序被 handler 处理，如果 handler 返回 false，则 event 不会被后续的 handler 处理，否则会一直被处理到最后一个 handler（捕获机制）；
- 上报的 event 可以被转换成不同的类型，即可以做聚合操作，也可以从 event 结构体转换成 json 类型；
- 多个不同的 ebpf 探针可以把 event 发送给同一个 handler，例如将文件访问信息和 process 执行信息合并成一个 event，获取每个文件访问的进程的 docker id，docker name，然后发送给 prometheus；
- handler 同样可以用来匹配对应的安全规则，在出现可能的安全风险的时候执行告警操作；

例如，上面所描述的 process 类就有对应的 handler：

- prometheus_event_handler;
- json_event_printer;
- plain_text_event_printer;

我们的安全风险分析和安全告警也可以基于对应的handler 实现，例如：

include\eunomia\sec_analyzer.h
```cpp
// base class for securiy rules
template<typename EVNET>
struct rule_base : event_handler<EVNET>
{
  std::shared_ptr<sec_analyzer> analyzer;
  rule_base(std::shared_ptr<sec_analyzer> analyzer_ptr) : analyzer(analyzer_ptr) {}
  virtual ~rule_base() = default;

  // return rule id if matched
  // return -1 if not matched
  virtual int check_rule(const tracker_event<EVNET> &e, rule_message &msg) = 0;
  void handle(tracker_event<EVNET> &e);
};
```

handler 的具体实现在 include\eunomia\model\event_handler.h 中。

我们设计了有多种类型的 handler，并通过模板实现：

- 接受单一线程的事件，并且把同样的事件传递给下一个handler，只有一个 next handler；（事件传递）
- 接受单一线程的事件，并且把不同的事件传递给下一个handler，只有一个 next handler；（类型转换，如做聚合操作）
- 接受单一线程的事件，并且把不同的事件传递给下一个handler，可以有多个 next handler；（多线程传递）
- 接受多个线程传递的事件，并且把事件传递给下一个handler，只有一个 next handler；这部分需要有多线程同步，可以用无锁队列实现；

所有的 handler 都继承自 event_handler_base，它规定了 handler 接受的事件类型：

include\eunomia\model\event_handler.h
```cpp
template <typename T>
struct event_handler_base
{
public:
    virtual ~event_handler_base() = default;
    virtual void handle(tracker_event<T> &e) = 0;
    virtual void do_handle_event(tracker_event<T> &e) = 0;
};
```

对于第一类的 handler，也是我们目前最经常用到的事件处理程序，它的模板如下：

```cpp
template <typename T>
struct event_handler : event_handler_base<T>
{
// ptr for next handler
std::shared_ptr<event_handler_base<T>> next_handler = nullptr;
public:
    virtual ~event_handler() = default;

    // implement this function to handle the event
    virtual void handle(tracker_event<T> &e) = 0;

    // add a next handler after this handler
    std::shared_ptr<event_handler<T>> add_handler(std::shared_ptr<event_handler<T>> handler)
    {
        next_handler = handler;
        return handler;
    }
    // do the handle event
    // pass the event to next handler
    void do_handle_event(tracker_event<T> &e)
    {   
        bool is_catched = false;
        try {
           is_catched = handle(e);
        } catch (const std::exception& error) {
            std::cerr << "exception: " << error.what() << std::endl;
            is_catched = true;
        }
        if (!is_catched && next_handler)
            next_handler->do_handle_event(e);
        return;
    }
};
```

例如 prometheus_event_handler，它就继承自 event_handler 类。每个探针上报的 ebpf 事件都会被转换成 tracker_event 类型，然后传递给 event_handler event_handler 类的 handle 方法就是对事件进行处理，并且传递给下一个 handler：handler 被组织成为单链表的形式（也可以是树或者有向无环图的形式），这样就可以实现事件的传递。

其他类型的 handler 可以参考 include\eunomia\model\event_handler.h 文件。

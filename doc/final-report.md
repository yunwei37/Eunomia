# 操作系统大赛：基于 eBPF 的容器监控工具 Eunomia 初赛报告

基于 eBPF 的轻量级 CloudNative Monitor 工具，用于容器安全性和可观察性

## 目录
<!-- TOC -->

- [目录](#目录)
- [目标描述](#目标描述)
  - [功能概述](#功能概述)
  - [Tutorial](#tutorial)
  - [容器追踪模块设计](#容器追踪模块设计)
    - [容器信息数据结构](#容器信息数据结构)
    - [容器追踪实现](#容器追踪实现)
  - [安全规则设计](#安全规则设计)
  - [seccomp: syscall准入机制](#seccomp-syscall准入机制)
- [开发计划](#开发计划)
  - [日程表](#日程表)
  - [未来的工作方向](#未来的工作方向)
- [比赛过程中的重要进展](#比赛过程中的重要进展)
  - [benchmark](#benchmark)
- [遇到的主要问题和解决方法](#遇到的主要问题和解决方法)
  - [如何设计 ebpf 挂载点](#如何设计-ebpf-挂载点)
  - [如何进行内核态数据过滤和数据综合](#如何进行内核态数据过滤和数据综合)
  - [如何定位容器元信息](#如何定位容器元信息)
  - [如何设计支持可扩展性的数据结构](#如何设计支持可扩展性的数据结构)
- [分工和协作](#分工和协作)
- [提交仓库目录和文件描述](#提交仓库目录和文件描述)
  - [项目仓库目录结构](#项目仓库目录结构)
  - [各目录及其文件描述](#各目录及其文件描述)
    - [bpftools目录](#bpftools目录)
    - [cmake目录](#cmake目录)
    - [doc目录](#doc目录)
    - [include目录](#include目录)
    - [libbpf目录](#libbpf目录)
    - [src目录](#src目录)
    - [test目录](#test目录)
    - [third_party目录](#third_party目录)
    - [tools目录](#tools目录)
    - [vmlinux目录](#vmlinux目录)

<!-- /TOC -->

## 目标描述

本项目由两部分组成：

* 一个零基础入门 `eBPF` 技术的教程实践和对应的命令行工具集，主要使用 `C/C++` 语言开发, 同时作为原型验证;
* 一个基于 `eBPF` 技术实现的用于监控容器的工具(**Eunomia**), 包含 `profile`、容器集群网络可视化分析、容器安全感知告警、一键部署、持久化存储监控等功能, 主要使用 C/C++ 语言开发, 力求为工业界提供覆盖容器全生命周期的轻量级开源监控解决方案;

### 功能概述

`Eunomia` 是一个使用 C/C++ 开发的基于eBPF的云原生监控工具，旨在帮助用户了解容器的各项行为、监控可疑的容器安全事件，力求为工业界提供覆盖容器全生命周期的轻量级开源监控解决方案。它使用 `Linux` `eBPF` 技术在运行时跟踪您的系统和应用程序，并分析收集的事件以检测可疑的行为模式。目前，它包含 `profile`、容器集群网络可视化分析*、容器安全感知告警、一键部署、持久化存储监控等功能。

* [X] 开箱即用：以单一二进制文件或镜像方式分发，一次编译，到处运行，一行代码即可启动，包含多种 ebpf 工具和多种监测点，支持多种输出格式（json, csv, etc) 并保存到文件；
* [X] 轻量级，高性能：编译成的二进制大小仅 `4MB`;
* [X] 通过 `ebpf` 自动收集容器和 k8s 相关元信息，并和多种指标相结合；
* [X] 可集成 `prometheus` 和 `Grafana`，作为监控可视化和预警平台；也可作为 `OpenTelemetry` 的 collector 使用；
* [X] 可自定义运行时安全预警规则, 并通过 prometheus 等实现监控告警;
* [X] 可以自动收集进程行为并通过 `seccomp`/`capability` 进行限制；
* [X] 提供远程的 http API 进行控制，实现 ebpf 跟踪器的热插拔，也可自行定制插件进行数据分析;
* [X] 其核心框架高度可扩展，可以非常轻松地集成其他的 libbpf ebpf C 程序；
* [X] 可通过 `graphql` 在远程发起 http 请求并执行监控工具，将产生的数据进行聚合后返回，用户可自定义运行时扩展插件进行在线数据分析；可外接时序数据库，如 `InfluxDB` 等，作为可选的信息持久化存储和数据分析方案；

除了收集容器中的一般系统运行时内核指标，例如系统调用、网络连接、文件访问、进程执行等，我们在探索实现过程中还发现目前对于 `lua` 和 `nginx` 相关用户态 `profile` 工具和指标可观测性开源工具存在一定的空白，但又有相当大的潜在需求；因此我们还计划添加一系列基于 uprobe 的用户态 `nginx/lua` 追踪器，作为可选的扩展方案；（这部分需求来自中科院开源之夏， APISIX 社区的选题）

和过去常用的 `BCC` 不同，`Eunomia` 基于 `Libbpf` + BPF CO-RE（一次编译，到处运行）开发。Libbpf 作为 BPF 程序加载器，接管了重定向、加载、验证等功能，BPF 程序开发者只需要关注 BPF 程序的正确性和性能即可。这种方式将开销降到了最低，且去除了庞大的依赖关系，使得整体开发流程更加顺畅。目前，我们已经发布了 `pre-release` 的版本，其中部分功能已经可以试用，只需下载二进制文件即可运行，


### Tutorial

`Eunomia` 的 `ebpf` 追踪器部分是从 `libbpf-tools` 中得到了部分灵感，但是目前关于 ebpf 的资料还相对零散且过时，这也导致了我们在前期的开发过程中走了不少的弯路。因此, 我们也提供了一系列教程，以及丰富的参考资料，旨在降低新手学习eBPF技术的门槛，试图通过大量的例程解释、丰富对 `eBPF、libbpf、bcc` 等内核技术和容器相关原理的认知，让后来者能更深入地参与到 ebpf 的技术开发中来。另外，`Eunomia` 也可以被单独编译为 C++ 二进制库进行分发，可以很方便地添加自定义 libbpf检查器，或者直接利用已有的功能来对 syscall 等指标进行监测，教程中也会提供一部分 `EUNOMIA` 扩展开发接口教程。

> 教程目前还在完善中。

1. [eBPF介绍](doc/tutorial/0_eBPF介绍.md)
2. [eBPF开发工具介绍: BCC/Libbpf，以及其他](doc/tutorial/1_eBPF开发工具介绍.md)
3. [基于libbpf的内核级别跟踪和监控: syscall, process, files 和其他](doc/tutorial/2_基于libbpf的内核级别跟踪和监控.md)
4. [基于uprobe的用户态nginx相关指标监控](doc/tutorial/3_基于uprobe的用户态nginx相关指标监控.md)
5. [seccomp权限控制](doc/tutorial/4_seccomp权限控制.md)
6. [上手Eunomia: 基于Eunomia捕捉内核事件](doc/tutorial/x_基于Eunomia捕捉内核事件.md)
7. [eBPF介绍与 libbpf 基础教程](doc/tutorial/tutorial.md)
8. [ebpf 跟踪器源码解析与使用教程](doc/trackers_doc/)


### 容器追踪模块设计

#### 容器信息数据结构
目前我们的容器追踪模块是基于进程追踪模块实现的，其数据结构为：
```c
struct container_event {
	struct process_event process;
	unsigned long container_id;
	char container_name[50];
};
```
容器追踪模块由`container_manager`控制，其中会有负责向docker发送http请求的client，以及存储收集到的信息的map，以及对应的hanler函数。
```c
class container_manager
{
 public:
  /// use process tracker to track the processes created in the container
  class container_tracking_handler : public event_handler<process_event>
  {
    container_manager &manager;

   public:
    void handle(tracker_event<process_event> &e);
    container_tracking_handler(container_manager &m) : manager(m)
    {
    }
  };

  template<typename EVENT>
      // use process tracker to track the processes created in the container
      class container_info_handler : public event_handler<EVENT>
  {
    container_manager &manager;

   public:
    void handle(tracker_event<EVENT> &e)
    {
      if (e.data.pid == 0) {
        return;
      }
      // no container info; get it
      e.ct_info = manager.get_container_info_for_pid(e.data.pid);
    }
    container_info_handler(container_manager &m) : manager(m){};
  };

  container_manager();
  // init the container info table
  void init();
  // get container info using the pid in root namespace
  container_info get_container_info_for_pid(int pid) const;

 private:
  // container client for getting container info
  class container_client
  {
   private:
    // for dockerd http api
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

  // for datas store in the container_info_map
  struct process_container_info_data
  {
    common_event common;
    container_info info;
  };

  // used to store container info
  // thread safe
  class container_info_map
  {
   private:
    // use rw lock to protect the map
    mutable std::shared_mutex mutex_;
    // pid -> container info
    std::unordered_map<int, process_container_info_data> container_info_map__;

   public:
    container_info_map() = default;
    // insert a container info into the map
    void insert(int pid, process_container_info_data info)
    {
      std::unique_lock<std::shared_mutex> lock(mutex_);
      container_info_map__[pid] = info;
    }
    // get a container info from the map
    std::optional<process_container_info_data> get(int pid) const
    {
      std::shared_lock<std::shared_mutex> lock(mutex_);
      auto ct_info_p = container_info_map__.find(pid);
      if (ct_info_p != container_info_map__.end())
      {
        return ct_info_p->second;
      }
      return std::nullopt;
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

  void get_all_process_info(void);
  // init the container info map for all running processes
  void update_container_map_data(void);
};
```

#### 容器追踪实现

容器追踪模块的ebpf代码服用了process追踪模块的ebpf代码，因此这里我们只介绍用户态下对数据处理的设计。   
在追踪器启动之初，程序会调用`update_container_map_data`函数，该函数会向docker发送http请求，获得当前已经在运行的容器中函数信息。该函数实现如下所示：
```c
void container_manager::update_container_map_data(void)
{
  auto response = client.list_all_containers();
  json containers_json = json::parse(response);
  for (const auto c : containers_json)
  {
    container_info info = { c["Id"], c["Names"][0], container_status_from_str(c["State"]) };

    json process_resp = json::parse(client.list_all_process_running_in_container(info.id));
    for (const auto p : process_resp["Processes"])
    {
      int pid = std::atoi(std::string(p[1]).c_str());
      int ppid = std::atoi(std::string(p[2]).c_str());

      auto map_data = info_map.get(pid);
      if (map_data)
      {
        // update existing data with new container info
        map_data->common.pid = pid;
        map_data->common.ppid = ppid;
        map_data->info = info;
        info_map.insert(pid, *map_data);
      }
      else
      {
        auto common_e = get_process_common_event(pid);
        common_e.ppid = ppid;
        info_map.insert(pid, process_container_info_data{ common_e, info });
      }
    }
  }
}
```
当内核态捕捉到进程的数据返回到用户态时，程序会进入handle函数。其具体实现为：
```c
void container_manager::container_tracking_handler::handle(tracker_event<process_event>& e)
{
  if (e.data.exit_event)
  {
    // process exit;
    manager.info_map.remove(e.data.common.pid);
  }
  else
  {
    // process start;
    auto this_info = manager.info_map.get(e.data.common.pid);
    if (this_info)
    {
      // find the pid and update the map
      manager.info_map.insert(
          e.data.common.pid, process_container_info_data{ .common = e.data.common, .info = this_info->info });
      return;
    }

    // find ppid info
    int ppid = e.data.common.ppid;
    auto pp_info = manager.info_map.get(ppid);
    if (pp_info)
    {
      // reinsert the info from the parent process
      auto data = *pp_info;
      if (!(data.common == e.data.common))
      {
        // not same namespace, update container info.
        spdlog::info(
            "different namespace from parent process, update info for pid {} name {}.", e.data.common.pid, e.data.comm);
        manager.update_container_map_data();
      }
      data.common = e.data.common;
      manager.info_map.insert(e.data.common.pid, data);
      return;
    }
    // no parent info, no this info
    spdlog::info("No parent info and this pid container info: pid {} name {}", e.data.common.pid, e.data.comm);
    // no info, insert os info to the map
    manager.info_map.insert(
        e.data.common.pid, process_container_info_data{ .common = e.data.common, .info = manager.os_info });
    // add new info to ppid
    manager.info_map.insert(
        e.data.common.ppid, process_container_info_data{ get_process_common_event(e.data.common.ppid), manager.os_info });
  }
  e.ct_info = manager.get_container_info_for_pid(e.data.common.pid);
}

```
如果进程处于退出状态，那么我们会直接将其从map中删去。否则，我们会判断
该进程是否存在于map中，如果存在则更新其信息。不存在则去检查其父进程是否存在于map中。如果其父进程存在，那么我们会判断父进程和本进程的namespace等信息是否一致。不一致时我们会认为有可能新的容器已经出现，此时我们会继续调用`update_container_map_data`函数，对容器内的进程进行检查。如果的确存在新的进程则会将其更新进入map中。

### 安全规则设计

目前安全告警部分还未完善，只有一个框架和 demo，我们需要对更多的安全相关规则，以及常见的容器安全风险情境进行调研和完善，然后再添加更多的安全分析。


- 安全分析和告警

  目前我们的安全风险等级主要分为三类（未来可能变化，我觉得这个名字不一定很直观）：

  include\eunomia\sec_analyzer.h
  ```cpp
  enum class sec_rule_level
  {
    event,
    warnning,
    alert,
    // TODO: add more levels?
  };
  ```

  安全规则和上报主要由 sec_analyzer 模块负责：

  ```cpp

  struct sec_analyzer
  {
    // EVNETODO: use the mutex
    std::mutex mutex;
    const std::vector<sec_rule_describe> rules;

    sec_analyzer(const std::vector<sec_rule_describe> &in_rules) : rules(in_rules)
    {
    }
    virtual ~sec_analyzer() = default;
    virtual void report_event(const rule_message &msg);
    void print_event(const rule_message &msg);

    static std::shared_ptr<sec_analyzer> create_sec_analyzer_with_default_rules(void);
    static std::shared_ptr<sec_analyzer> create_sec_analyzer_with_additional_rules(const std::vector<sec_rule_describe> &rules);
  };

  struct sec_analyzer_prometheus : sec_analyzer
  {
    prometheus::Family<prometheus::Counter> &eunomia_sec_warn_counter;
    prometheus::Family<prometheus::Counter> &eunomia_sec_event_counter;
    prometheus::Family<prometheus::Counter> &eunomia_sec_alert_counter;

    void report_prometheus_event(const struct rule_message &msg);
    void report_event(const rule_message &msg);
    sec_analyzer_prometheus(prometheus_server &server, const std::vector<sec_rule_describe> &rules);

    static std::shared_ptr<sec_analyzer> create_sec_analyzer_with_default_rules(prometheus_server &server);
    static std::shared_ptr<sec_analyzer> create_sec_analyzer_with_additional_rules(const std::vector<sec_rule_describe> &rules, prometheus_server &server);
  };
  ```

  我们通过 sec_analyzer 类来保存所有安全规则以供查询，同时以它的子类 sec_analyzer_prometheus 完成安全事件的上报和告警。具体的告警信息发送，可以由 prometheus 的相关插件完成，我们只需要提供一个接口。由于 rules 是不可变的，因此它在多线程读条件下是线程安全的。

- 安全规则实现

  我们的安全风险分析和安全告警规则基于对应的handler 实现，例如：

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
    void handle(tracker_event<EVNET> &e)
    {
      if (!analyzer)
      {
        std::cout << "analyzer is null" << std::endl;
      }
      struct rule_message msg;
      int res = check_rule(e, msg);
      if (res != -1)
      {
        analyzer->report_event(msg);
      }
    }
  };
  ```

  这个部分定义了一个简单的规则基类，它对应于某一个 ebpf 探针上报的事件进行过滤分析，以系统调用上报的事件为例：

  ```cpp
  // syscall rule:
  //
  // for example, a process is using a dangerous syscall
  struct syscall_rule_checker : rule_base<syscall_event>
  {
    syscall_rule_checker(std::shared_ptr<sec_analyzer> analyzer_ptr) : rule_base(analyzer_ptr)
    {}
    int check_rule(const tracker_event<syscall_event> &e, rule_message &msg);
  };
  ```

  其中的 check_rule 函数实现了对事件进行过滤分析，如果事件匹配了规则，则返回规则的 id，否则返回 -1：关于 check_rule 的具体实现，请参考：src\sec_analyzer.cpp

  除了通过单一的 ebpf 探针上报的事件进行分析之外，通过我们的 handler 机制，我们还可以综合多种探针的事件进行分析，或者通过时序数据库中的查询进行分析，来发现潜在的安全风险事件。

-  其他
  
  除了通过规则来实现安全风险感知，我们还打算通过机器学习等方式进行进一步的安全风险分析和发现。

### seccomp: syscall准入机制

Seccomp(全称：secure computing mode)在2.6.12版本(2005年3月8日)中引入linux内核，将进程可用的系统调用限制为四种：read，write，_exit，sigreturn。最初的这种模式是白名单方式，在这种安全模式下，除了已打开的文件描述符和允许的四种系统调用，如果尝试其他系统调用，内核就会使用SIGKILL或SIGSYS终止该进程。Seccomp来源于Cpushare项目，Cpushare提出了一种出租空闲linux系统空闲CPU算力的想法，为了确保主机系统安全出租，引入seccomp补丁，但是由于限制太过于严格，当时被人们难以接受。

尽管seccomp保证了主机的安全，但由于限制太强实际作用并不大。在实际应用中需要更加精细的限制，为了解决此问题，引入了Seccomp – Berkley Packet Filter(Seccomp-BPF)。Seccomp-BPF是Seccomp和BPF规则的结合，它允许用户使用可配置的策略过滤系统调用，该策略使用Berkeley Packet Filter规则实现，它可以对任意系统调用及其参数（仅常数，无指针取消引用）进行过滤。Seccomp-BPF在3.5版（2012年7月21日）的Linux内核中（用于x86 / x86_64系统）和Linux内核3.10版（2013年6月30日）被引入Linux内核。

seccomp在过滤系统调用(调用号和参数)的时候，借助了BPF定义的过滤规则，以及处于内核的用BPF language写的mini-program。Seccomp-BPF在原来的基础上增加了过滤规则，大致流程如下：

<img src="./imgs/seccomp.png" weight=100% height=100%>

## 开发计划

### 日程表

阶段一：学习ebpf相关技术栈（3.10~4.2）

* [X] 入门ebpf技术栈
* [X] 调研、学习 `bcc`
* [X] 调研、学习 `libbpf` 、`libbpf-bootstrap`
* [X] 调研、学习 `seccomp`
* [X] 输出调研文档

阶段二：项目设计（4.3~4.10）

* [X] 与mentor讨论项目需求、并设计功能模块
* [X] 输出系统设计文档
* [X] 输出模块设计文档

阶段三：开发迭代（4.10~6.1）

* [X] 实现进程信息监控（pid、ppid等）
* [X] 实现系统调用信息监控
* [X] 实现进程间通信监控
* [X] 实现tcp（ipv4、ipv6）通信监控
* [X] 实现监控信息存储功能（csv或json格式）
* [X] 完成了系统的原型验证功能
* [X] 基于上述功能，实现命令行调用，完成版本v0.1
* [X] 输出开发v0.1日志文档
* [X] 实现进程id与容器id映射，进程信息过滤
* [X] 添加“seccomp”功能
* [x] 基于上述新增功能，迭代版本v0.2
* [X] 输出开发v0.2日志文档
* [x] 添加可视化模块: prometheus and grafana
* [X] add more tools from libbpf-tools
* [X] 基于上述新增功能，迭代版本v0.3
* [X] 输出开发v0.3日志文档
* [ ] 后续更新迭代

阶段四：开发测试（6.2~6.16）

* [X] lsm support
* [X] add more rules
* [X] 设计测试场景（分别针对基础功能、权限控制、安全逃逸场景）
* [X] 搭建测试环境
* [X] 测试-开发
* [X] 输出测试文档

阶段五：项目文档完善（6.17~7.1）

* [X] 完善开发文档
* [X] 完善教程文档
* [X] 完善labs

### 未来的工作方向

在未来我们计划继续按照上述日程表，完成我们未完成的工作，同时不断优化代码，使得Eunomia能成为一个具有较大使用价值的工具。主要有如下几个方向：

- 完善单元测试和测试场景，并且提供更完整的 benchmark；
- 完善教程文档；
- 完善安全规则和安全分析告警模块，这一部分目前只是个雏形；
- 添加更多的 ebpf 探针，并且支持自定义探针；
- 完善 http 在线分析模块；
- ......

## 比赛过程中的重要进展

- 2022.5.1 首段代码push，实现了对系统调用的成功追踪
- 2022.5.15 完成了五大追踪模块的ebpf代码和简易用户态代码
- 2022.5.17 重构用户态代码，引入简易命令行控制
- 2020.5.20 正式定名Eunomia，该名字的原意是古希腊神话中的一位司管明智，法律与良好秩序女神。我们希望本工具也能在容器安全检测和可观测性中发挥到这样的作用。
- 2022.5.22 将CMake引入本工程，提高了项目编译的速度
- 2022.5.23 开始集成 Prometheus 模块进入工程
- 2022.5.24 重构用户态代码，基本确定命令行控制形式
- 2022.5.28 将日志记录工具spdlog引入本工程
- 2022.6.1 prometheus 和 Grafana 模块集成完成，设计相关 dashboard
- 2022.6.3 完成了 sec_analyzer 模块，对安全风险事件进行分析,发布第一个版本V0.05
- 2022.6.4 整理初赛文档；
- 2022.8.1 添加静态分析和运行时分析，保证安全性与可用性
- 2022.8.7 发布第一个大版本V0.2
- 2022.8.8 接入一系列新的trakcer
- 2022.8.9 发布版本V0.25
- 2022.8.14 整理决赛报告，完善文档


### benchmark

使用 top 查看 eunomia 的内存和cpu占用情况

![top](./imgs/top2.png)

目前有一些简单的性能对比，使用 openresty 在本机上启动一个网络简单的服务，并且使用 wrk 进行压力测试。测试环境：

```
Linux ubuntu 5.13.0-44-generic #49~20.04.1-Ubuntu SMP x86_64 GNU/Linux
4 核，12 GB 内存：
```

这是未开启 eunomia server 的情况：

![no](imgs/openresty_no_eunomia2.png)

这是启动 eunomia server 后的情况，使用默认配置并启用 process/container、tcp、files、ipc 等探针，在同样环境下进行测试：

![no](imgs/openresty_with_eunomia2.png)

可以观测到仅有大约 2% 的性能损耗。

> OpenResty® 是一个基于 Nginx 与 Lua 的高性能 Web 平台，其内部集成了大量精良的 Lua 库、第三方模块以及大多数的依赖项。用于方便地搭建能够处理超高并发、扩展性极高的动态 Web 应用、Web 服务和动态网关。web开发人员可以使用lua编程语言，对核心以及各种c模块进行编程，可以利用openresty快速搭建超1万并发高性能web应用系统。这里的 benchmark 参考了：https://openresty.org/en/benchmark.html

目前还没有比较完善的 benchmark 测试和性能分析，这是我们接下来要完善的内容。

## 遇到的主要问题和解决方法

### 如何设计 ebpf 挂载点

如何设计挂载点是ebpf程序在书写时首先需要考虑的问题。ebpf程序是事件驱动的，即只有系统中发生了我们预先规定的事件，我们的程序才会被调用。因此，ebpf挂载点的选择直接关系到程序能否在我们需要的场合下被启动。

我们在选择挂载点时，首先需要明白的是我们需要在什么情况下触发处理函数，然后去寻找合适的挂载点。ebpf的挂载点有多种类型，较为常用的挂载点是 `tracepoint` ， `k/uprobe` ， `lsm` 等。

`tracepoint` 是一段静态的代码，以打桩的形式存在于程序源码中，并向外界提供钩子以挂载。一旦处理函数挂载到了钩子上，那么当钩子对应的事件发生时，处理函数就会被调用。由于 `tracepoint` 使用较为方便，且覆盖面广，ABI也较为稳定，他是我们设计挂载点的一个重要考虑对象。目前Linux已经有1000多个tracepoint可供选择，其支持的所有类型可以在 `/sys/kernel/debug/tracing/events/` 目录下看到，而至于涉及到的参数格式和返回形式，用户可以使用 `cat` 命令，查看对应 `tracepoint` 事件下的format文件得到。

如下便是`sched_process_exec`事件的输出格式。

![](./imgs/report/tracepoint_example1.png)

用户也可以直接访问 `tracepoint` 的源码获得更多信息。在Linux源码的 `./include/trace/events` 目录下，用户可以看到Linux中实现tracepoint的源码。  

`k/uprobe` 是Linux提供的，允许用户动态插桩的方式。由于 `tracepoint` 是静态的，如果用户临时需要对一些其不支持的函数进行追踪，就无法使用 `tracepoint` ，而 `k/uprobe` 允许用户事实对内核态/用户态中某条指令进行追踪。用户在指定了该指令的位置并启用 `k/uprobe` 后，当程序运行到该指令时，内核会自动跳转到我们处理代码，待处理完成后返回到原处。相较于 `tracepoint` ， `k/uprobe` 更为灵活，如果你需要追踪的指令不被 `tracepoint` 所支持，可以考虑使用 `k/uprobe`。

`lsm` 是Linux内核安全模块的一套框架，其本质也是插桩。相较于`tracepoint` ，`lsm` 主要在内核安全的相关路径中插入了hook点。因此如果你希望你的代码检测一些和安全相关的内容，可以考虑使用 `lsm` 。其所有钩子的定义在Linux源码的 `./include/linux/lsm_hook_defs.h` 中，你可以从中选择初你所需要的hook点。


### 如何进行内核态数据过滤和数据综合

如果直接将所有捕获到的数据直接传递到用户态的话，将会带来很大的开销，ebpf 程序一个重要的特征就是能在内核态进行数据过滤和综合。我们设计了一系列数据综合和过滤模式，如：

- 根据次数进行统计，如 syscall 统计每个进程调用 syscall 的次数并存储在map中，而不是直接上报；
- 根据 pid、namespace、cgroup 进行过滤；
- process 短于一定时间间隔的不予统计；
- 根据一定时间进行统计采样，如 files

### 如何定位容器元信息

在程序开始，我们调用 `docker ps -q` 命令获得当前所有正在运行的容器ID。

之后我们开始遍历这些ID，并对每一个ID调用 `docker top id` 命令,获得容器中的所有所有进程信息，并且将这些信息以键值对的形式存储到哈希map上。之后我们会在 `sched_process_exec` 和 `sched_process_exit` 的两个点挂载基于ebpf的处理函数，捕获进程信息。

如果捕获的进程与其父进程存在 namespace 变化的情况，那么我们就会重复一次开始的工作，判断是否有新的容器产生。

如果有，则将其添加到哈希map中。如果其父进程已经存在于哈希map中，那么我们就认为此进程也是一个容器相关进程，也将其存储到哈希map中。在进程退出时，我们则需要检查其是否存在于哈希map中，如果存在则需要将其删除。


### 如何设计支持可扩展性的数据结构

首先尽可能降低各个模块的耦合性，这样在修改时可以较为方便地完成改动。其次，在最初设计时为未来可能的扩展预留位置。这部分主要是 tracker 和 handler 的设计，具体可以参考我们的设计文档部分：[#44-ebpf-探针设计](#44-ebpf-探针设计)

## 分工和协作

现在我们大部分的分工方式是在 gitlab 上面的 issue 中进行分配，并且且通过 issue 完成项目质量管理和追踪。参考：[issues](https://gitlab.eduxiji.net/zhangdiandian/project788067-89436/-/issues)

- 郑昱笙同学：负责项目架构设计和调研，负责了 process、files、syscall 相关探针设计；
- 张典典同学：主要负责了seccomp模块、可视化以及部分quickstart的工作
- 濮雯旭同学：主要负责了container和ipc追踪模块的撰写以及后期用户态代码中与命令行控制相关的重构工作


## 提交仓库目录和文件描述


### 项目仓库目录结构

本仓库的主要目录结构如下所示：   

  ```
 ├─bpftools       - ebpf内核态代码
  |  ├─bindsnoop
  |  ├─biolatency
  |  ├─biopattern
  |  ├─biosnoop 
  |  ├─biostacks
  |  ├─bitesize
  |  ├─capable  
  │  ├─container  
  │  ├─files
  │  ├─funclatency  
  │  ├─ipc
  │  ├─llcstat
  │  ├─memleak
  │  ├─mountsnoop
  │  ├─oomkill
  │  ├─opensnoop  
  │  ├─process
  │  ├─profile  
  │  ├─seccomp  
  │  ├─syscall
  |  ├─syscount 
  |  ├─sigsnoop
  |  ├─tcpconnect 
  |  ├─tcprtt
  │  └─tcpconnlat 
  ├─cmake  
  ├─doc           - 项目开发文档
  │  ├─develop_doc   
  │  ├─imgs  
  │  └─tutorial  
  ├─include       
  │   └─eunomia   - 项目主要头文件
  │       └─model  
  ├─libbpf  
  ├─src           - 项目主要代码
  ├─test          - 项目单元测试和集成测试
  │   └─src  
  ├─third_party  
  │       └─prometheus-cpp  
  ├─tools  
  └─vmlinux  
  ```
### 各目录及其文件描述

#### bpftools目录

本目录内的所有文件均为基于ebpf开发的内核态监视代码，
共有7个子目录，子目录名表示了子目录内文件所实现的模块。比如process子目录代表了其中的文件，主要实现了进程追踪方面的ebpf内核态代码，其他子目录同理。

#### cmake目录

本项目使用cmake进行编译，本目录中的所有文件都是本项目cmake
的相关配置文件。

#### doc目录

本目录内的所有文件为与本项目相关的文档，其中develop_doc目录为开发文档，其中记录了本项目开发的各种详细信息。tutorial目录为本项目为所有想进行ebpf开发的同学所设计的教学文档，其中会提供一些入门教程，方便用户快速上手。imgs目录为开发文档和教学文档中所需要的一些
图片。

#### include目录

本项目中用户态代码的头文件均会存放在本目录下。eunomia子目录中存放的是各个模块和所需要的头文件，eunomia下的model子目录存放的是各个头文件中的一些必要结构体经过抽象后的声明。

#### libbpf目录

该目录为libbpf-bootstrap框架中自带的libbpf头文件。

#### src目录

该目录主要记录了各个模块的用户态代码cpp文件。

#### test目录

本目录主要包括了对各个模块的测试代码。

#### third_party目录

本模块为Prometheus库所需的依赖。

#### tools目录

本模块主要包含了一些项目所需要的脚本。

#### vmlinux目录

本目录主要是libbpf-bootstrap框架自带的vmlinux头文件。

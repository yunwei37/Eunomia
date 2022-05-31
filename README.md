# Eunomia

A lightweight eBPF-based CloudNative Monitor tool for Container Security and Observability

> NOTE: This repo is under heavily development and `NOT YET COMPLETE`, it shall not be used in product environments now.

[![Actions Status](https://github.com/filipdutescu/modern-cpp-template/workflows/Ubuntu/badge.svg)](https://github.com/filipdutescu/modern-cpp-template/actions)
[![codecov](https://codecov.io/gh/filipdutescu/modern-cpp-template/branch/master/graph/badge.svg)](https://codecov.io/gh/filipdutescu/modern-cpp-template)
[![GitHub release (latest by date)](https://img.shields.io/github/v/release/yunwei37/Eunomia)](https://github.com/filipdutescu/modern-cpp-template/releases)

We have a mirror of the source code on [GitHub](https://github.com/yunwei37/Eunomia) which runs CI. We also have a mirror on [GitLab](https://gitlab.eduxiji.net/zhangdiandian/project788067-89436), for faster access in China and OS comp.

<!-- TOC -->

- [Eunomia](#eunomia)
  - [What is Eunomia](#what-is-eunomia)
    - [Describe](#describe)
    - [Tutorial](#tutorial)
  - [Quickstart](#quickstart)
    - [Prequest](#prequest)
    - [run as binary](#run-as-binary)
    - [Docker, Prometheus and Grafana](#docker-prometheus-and-grafana)
    - [Prometheus and rafana 的效果如图：](#prometheus-and-rafana-的效果如图)
    - [build On Linux](#build-on-linux)
  - [Why is eBPF](#why-is-ebpf)
  - [Architecture](#architecture)
  - [Roadmap](#roadmap)
  - [Documents](#documents)
  - [Reference](#reference)
- [Contact](#contact)

<!-- /TOC -->

## What is Eunomia

### Describe

`Eunomia` 是一个使用 C/C++ 开发的基于eBPF的云原生监控工具，旨在帮助用户了解容器的各项行为、监控可疑的容器安全事件，力求为工业界提供覆盖容器全生命周期的轻量级开源监控解决方案。它使用 `Linux` `eBPF` 技术在运行时跟踪您的系统和应用程序，并分析收集的事件以检测可疑的行为模式。目前，它包含 `profile`、容器集群网络可视化分析*、容器安全感知告警、一键部署、持久化存储监控等功能。

* [X] 开箱即用：以单一二进制文件或 `docker` 镜像方式分发，一次编译，到处运行，一行代码即可启动，包含多种 ebpf 工具和多种监测点，支持多种输出格式（json, csv, etc)；
* [X] 可集成 `prometheus` 和 `Grafana`，作为监控可视化和预警平台；
* [X] 可自定义运行时安全预警规则，也可以自动收集进程系统调用行为并通过 seccomp 进行限制；
* [ ] 可外接时序数据库，如 `InfluxDB` 等，作为可选的信息持久化存储方案；
* [ ] 可通过 `graphql` 在远程发起 http 请求并执行监控工具，将产生的数据进行聚合后返回，用户可自定义运行时扩展插件进行在线数据分析；

除了收集容器中的一般系统运行时内核指标，例如系统调用、网络连接、文件访问、进程执行等，我们在探索实现过程中还发现目前对于 `lua` 和 `nginx` 相关用户态 `profile` 工具和指标可观测性开源工具存在一定的空白，但又有相当大的潜在需求；因此我们还计划添加一系列基于 uprobe 的用户态 `nginx/lua` 追踪器，作为可选的扩展方案；

和过去常用的 `BCC` 不同，`Eunomia` 基于 `Libbpf` + BPF CO-RE（一次编译，到处运行）开发。Libbpf 作为 BPF 程序加载器，接管了重定向、加载、验证等功能，BPF 程序开发者只需要关注 BPF 程序的正确性和性能即可。这种方式将开销降到了最低，且去除了庞大的依赖关系，使得整体开发流程更加顺畅。

### Tutorial

`Eunomia` 的 `ebpf` 追踪器部分是从 `libbpf-tools` 中得到了部分灵感，但是目前关于 ebpf 的资料还相对零散且过时，这也导致了我们在前期的开发过程中走了不少的弯路。因此, 我们也提供了一系列教程，以及丰富的参考资料，旨在降低新手学习eBPF技术的门槛，试图通过大量的例程解释、丰富对 `eBPF、libbpf、bcc` 等内核技术和容器相关原理的认知，让后来者能更深入地参与到 ebpf 的技术开发中来。另外，`Eunomia` 也可以被单独编译为 C++ 二进制库进行分发，可以很方便地添加自定义 libbpf检查器，或者直接利用已有的功能来对 syscall 等指标进行监测，教程中也会提供一部分 `EUNOMIA` 扩展开发接口教程。

> 教程目前还在完善中。

1. [eBPF介绍](doc/tutorial/0_eBPF介绍.md)
2. [eBPF开发工具介绍: BCC/Libbpf，以及其他](doc/tutorial/1_eBPF开发工具介绍.md)
3. [基于libbpf的内核级别跟踪和监控: syscall, process, files 和其他](doc/tutorial/2_基于libbpf的内核级别跟踪和监控.md)
4. [基于uprobe的用户态nginx相关指标监控](doc/tutorial/3_基于uprobe的用户态nginx相关指标监控.md)
5. [seccomp权限控制](doc/tutorial/4_seccomp权限控制.md)
6. [上手Eunomia: 基于Eunomia捕捉内核事件](doc/tutorial/x_基于Eunomia捕捉内核事件.md)

## Quickstart

### Prequest

Your Kconfig should contain the options below

- Compile options
  ```
  CONFIG_DEBUG_INFO_BTF=y
  CONFIG_DEBUG_INFO=y
  ```
- The suggested kernel version is `5.13` or higher.

### run as binary

you can use our pre-compiled binary to start a prometheus exporter:

```
./eunomia server
```

This will enable our core ebpf trackers including `process`, `tcp`, `syscall` and `files`, it will also start our security engine to detect potential security issues. For more details, you can refer to our doc.

Alternatively, you can simply use eunomia to run a single ebpf tracker, for example:

```
./eunomia run files
```

will trace all files read or write in the system at a defaut interval of 3s, and print the result:

```sh
[2022-05-28 11:23:10.699] [info] start eunomia...
[2022-05-28 11:23:10.699] [info] start ebpf tracker...
[2022-05-28 11:23:10.699] [info] start prometheus server...
[2022-05-28 11:23:10.699] [info] press 'Ctrl C' key to exit...
[2022-05-28 11:23:13.785] [info] pid    read_bytes      read count      write_bytes     write count     comm    type    tid     filename
[2022-05-28 11:23:13.785] [info] 34802  2048            1               0               0               ps      R       34802   status
[2022-05-28 11:23:13.785] [info] 34802  2048            1               0               0               ps      R       34802   stat
[2022-05-28 11:23:13.785] [info] 34802  2048            1               0               0               ps      R       34802   status
....
```

You can also use `--container-id` to trace a container, or use `toml` config file. You can specify the interval of tracing or sampling, the output format, the log level, etc.

we have provide five default trackers, `process`, `tcp`, `syscall`, `ipc` and `files`. You can also add your own trackers from libbpf-tools easily.

for more details, see: [usage.md](doc/usage.md)

### Docker, Prometheus and Grafana

> TODO: docker file


### Prometheus and rafana 的效果如图：

<div  align="center">  
 <img src="doc/imgs/prometheus1.png" alt="eunomia_prometheus1" align=center />
 <p>文件读取的byte数</p>
 <img src="doc/imgs/prometheus2.png" alt="eunomia_prometheus1" align=center />
  <p>文件读取的系统调用次数</p>
 <img src="doc/imgs/prometheus3.png" alt="eunomia_prometheus1" align=center />
 <p>对于容器中进程的跟踪结果，记录开始和结束时间</p>
</div>

- 对于详细的 Prometheus 监控指标文档，请参考：[prometheus_metrics.md](doc/prometheus_metrics.md)

- 关于如何集成 Prometheus 和 Grafane，请参考：[intergration.md](doc/intergration.md)

### build On Linux

You may need to install `libcurl`, `libelf-dev` `clang` and `gtest` as deps. On `Debian/Ubuntu`, just run

```
make install-deps
```

We used `C++20` as standard， so you need a compiler that supports C++20, for example, `GCC` > 10.0

Makefile build:

```shell
git submodule update --init --recursive       # check out deps
make install
```

## Why is eBPF

eBPF是一项革命性的技术，可以在Linux内核中运行沙盒程序，而无需更改内核源代码或加载内核模块。通过使Linux内核可编程，基础架构软件可以利用现有的层，从而使它们更加智能和功能丰富，而无需继续为系统增加额外的复杂性层。

* 优点：低开销

  eBPF 是一个非常轻量级的工具，用于监控使用 Linux 内核运行的任何东西。虽然 eBPF 程序位于内核中，但它不会更改任何源代码，这使其成为泄露监控数据和调试的绝佳伴侣。eBPF 擅长的是跨复杂系统实现无客户端监控。 
* 优点：安全

  解决内核观测行的一种方法是使用内核模块，它带来了大量的安全问题。而eBPF 程序不会改变内核，所以您可以保留代码级更改的访问管理规则。此外，eBPF 程序有一个验证阶段，该阶段通过大量程序约束防止资源被过度使用，保障了运行的ebpf程序不会在内核产生安全问题。
* 优点：精细监控、跟踪

  eBPF 程序能提供比其他方式更精准、更细粒度的细节和内核上下文的监控和跟踪标准。并且eBPF监控、跟踪到的数据可以很容易地导出到用户空间，并由可观测平台进行可视化。 
* 缺点：很新

  eBPF 仅在较新版本的 Linux 内核上可用，这对于在版本更新方面稍有滞后的组织来说可能是令人望而却步的。如果您没有运行 Linux 内核，那么 eBPF 根本不适合您。

## Architecture

<div  align="center">  
 <img src="doc/imgs/architecture.jpg" width = "600" height = "400" alt="eunomia_architecture" align=center />
 <p>系统架构</p>
</div>

关于详细的系统架构设计和模块划分，请参考 [系统设计文档](doc/design_doc)

## Roadmap

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

* [ ] graphql for extentions
* [ ] lsm support
* [ ] add more rules
* [ ] 设计测试场景（分别针对基础功能、权限控制、安全逃逸场景）
* [X] 搭建测试环境
* [ ] 测试-开发
* [ ] 输出测试文档

阶段五：项目文档完善（6.17~7.1）

* [ ] 完善开发文档
* [ ] 完善教程文档
* [ ] 完善labs

## Documents

Eunomia的完整文档如下

- [develop documents](doc/develop_doc)
- [design documents](doc/design_doc)
- [tutorial](doc/tutorial)

## Reference

* [基于 eBPF 实现容器运行时安全](https://mp.weixin.qq.com/s/UiR8rjTt2SgJo5zs8n5Sqg)
* [基于ebpf统计docker容器网络流量](https://blog.csdn.net/qq_32740107/article/details/110224623)
* [BumbleBee: Build, Ship, Run eBPF tools](https://www.solo.io/blog/solo-announces-bumblebee/)
* [Container traffic visibility library based on eBPF](https://github.com/ntop/libebpfflow)
* [why-libbpf-bootstrap](https://nakryiko.com/posts/libbpf-bootstrap/#why-libbpf-bootstrap)
* [bpf-core-reference-guide](https://nakryiko.com/posts/bpf-core-reference-guide/)
* [bcc to libbpf](https://nakryiko.com/posts/bcc-to-libbpf-howto-guide/#setting-up-user-space-parts)
* good intro for trace point and kprobe in ebpf
  https://www.iserica.com/posts/brief-intro-for-tracepoint/
  https://www.iserica.com/posts/brief-intro-for-kprobe/
* other
  https://lockc-project.github.io/book/index.html
  https://github.com/willfindlay/bpfcontain-rs
* user space uprobe:
  [an-ebpf-overview-part-5-tracing-user-processes](https://www.collabora.com/news-and-blog/blog/2019/05/14/an-ebpf-overview-part-5-tracing-user-processes/)
* ebpf secomp
  [how_does_the_bpf_recorder_work_](https://developers.redhat.com/articles/2021/12/16/secure-your-kubernetes-deployments-ebpf#how_does_the_bpf_recorder_work_)
  [recorder.bpf.c](https://github.com/kubernetes-sigs/security-profiles-operator/blob/main/internal/pkg/daemon/bpfrecorder/bpf/recorder.bpf.c)
* [libbpf-tools](https://github.com/iovisor/bcc/tree/master/libbpf-tools)

# Contact

**成员**

指导老师：程泽睿志（华为）李东昂（浙江大学）

学生：郑昱笙（yunwei37: 1067852565@qq.com），濮雯旭，张典典

# 项目设计报告: Eunomia

基于 eBPF 的轻量级 CloudNative Monitor 工具，用于容器安全性和可观察性

## 1. 目录
<!-- TOC -->

- [项目设计报告: Eunomia](#项目设计报告-eunomia)
  - [1. 目录](#1-目录)
  - [2. 目标描述](#2-目标描述)
    - [2.1. 功能概述](#21-功能概述)
    - [2.2. Tutorial](#22-tutorial)
  - [3. 比赛题目分析和相关资料调研](#3-比赛题目分析和相关资料调研)
    - [3.1. 题目描述](#31-题目描述)
    - [3.2. 赛题分析](#32-赛题分析)
    - [3.3. 相关资料调研](#33-相关资料调研)
      - [3.3.1. ebpf](#331-ebpf)
      - [3.3.2. ebpf 开发工具技术选型](#332-ebpf-开发工具技术选型)
      - [3.3.3. 容器可观测性](#333-容器可观测性)
      - [3.3.4. 信息可视化展示](#334-信息可视化展示)
      - [3.3.5. 容器运行时安全](#335-容器运行时安全)
  - [4. 系统框架设计](#4-系统框架设计)
    - [4.1. 系统设计](#41-系统设计)
    - [4.2. 模块设计](#42-模块设计)
    - [4.3. 功能设计](#43-功能设计)
    - [4.4. ebpf 主要观测点](#44-ebpf-主要观测点)
    - [4.5. ebpf 可观测信息设计](#45-ebpf-可观测信息设计)
    - [4.6. 重要数据结构设计](#46-重要数据结构设计)
    - [4.7. 安全规则设计](#47-安全规则设计)
  - [5. 开发计划](#5-开发计划)
    - [5.1. 日程表](#51-日程表)
    - [5.2. 未来的工作方向](#52-未来的工作方向)
  - [6. 比赛过程中的重要进展](#6-比赛过程中的重要进展)
  - [7. 系统测试情况](#7-系统测试情况)
    - [7.1. 快速上手](#71-快速上手)
    - [7.2. 命令行测试情况](#72-命令行测试情况)
    - [7.3. 容器测试情况](#73-容器测试情况)
    - [7.4. 信息可视化测试情况： prometheus and grafana](#74-信息可视化测试情况-prometheus-and-grafana)
    - [7.5. CI/持续集成](#75-ci持续集成)
  - [8. 遇到的主要问题和解决方法](#8-遇到的主要问题和解决方法)
    - [8.1. 如何设计 ebpf 挂载点](#81-如何设计-ebpf-挂载点)
    - [8.2. 如何进行内核态数据过滤和数据综合](#82-如何进行内核态数据过滤和数据综合)
    - [8.3. 如何定位容器元信息](#83-如何定位容器元信息)
    - [8.4. 如何设计支持可扩展性的数据结构](#84-如何设计支持可扩展性的数据结构)
  - [9. 分工和协作](#9-分工和协作)
  - [10. 提交仓库目录和文件描述](#10-提交仓库目录和文件描述)
    - [10.1. 项目仓库目录结构](#101-项目仓库目录结构)
    - [10.2. 各目录及其文件描述](#102-各目录及其文件描述)
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
  - [11. 比赛收获](#11-比赛收获)
  - [12. 附录](#12-附录)
    - [12.1. Prometheus 观测指标](#121-prometheus-观测指标)
  - [Process Metrics](#process-metrics)
    - [Metrics List](#metrics-list)
    - [Labels List](#labels-list)
  - [files Metrics](#files-metrics)
    - [Metrics List](#metrics-list-1)
    - [Labels List](#labels-list-1)
  - [Tcp Connect Metrics](#tcp-connect-metrics)
    - [Metrics List](#metrics-list-2)
    - [Labels List](#labels-list-2)
  - [Syscall Metrics](#syscall-metrics)
    - [Metrics List](#metrics-list-3)
    - [Labels List](#labels-list-3)
  - [Security Event Metrics](#security-event-metrics)
    - [Metrics List](#metrics-list-4)
    - [Labels List](#labels-list-4)
  - [Service Metrics](#service-metrics)
    - [Metrics List](#metrics-list-5)
    - [Labels List](#labels-list-5)
  - [PromQL Example](#promql-example)
    - [12.2. 命令行工具帮助信息](#122-命令行工具帮助信息)

<!-- /TOC -->

## 2. 目标描述

本项目由两部分组成：

* 一个零基础入门 `eBPF` 技术的教程实践和对应的命令行工具集，主要使用 `C/C++` 语言开发, 同时作为原型验证;
* 一个基于 `eBPF` 技术实现的用于监控容器的工具(**Eunomia**), 包含 `profile`、容器集群网络可视化分析、容器安全感知告警、一键部署、持久化存储监控等功能, 主要使用 Go 语言开发, 力求为工业界提供覆盖容器全生命周期的轻量级开源监控解决方案;

### 2.1. 功能概述

`Eunomia` 是一个使用 C/C++ 开发的基于eBPF的云原生监控工具，旨在帮助用户了解容器的各项行为、监控可疑的容器安全事件，力求为工业界提供覆盖容器全生命周期的轻量级开源监控解决方案。它使用 `Linux` `eBPF` 技术在运行时跟踪您的系统和应用程序，并分析收集的事件以检测可疑的行为模式。目前，它包含 `profile`、容器集群网络可视化分析*、容器安全感知告警、一键部署、持久化存储监控等功能。

* [X] 开箱即用：以单一二进制文件或 `docker` 镜像方式分发，一次编译，到处运行，一行代码即可启动，包含多种 ebpf 工具和多种监测点，支持多种输出格式（json, csv, etc) 并保存到文件；
* [X] 通过 `ebpf` 自动收集容器相关元信息，并和多种指标相结合；
* [X] 可集成 `prometheus` 和 `Grafana`，作为监控可视化和预警平台；
* [X] 可自定义运行时安全预警规则, 并通过 prometheus 等实现监控告警; 
* [X] 可以自动收集进程系统调用行为并通过 seccomp 进行限制；
* [ ] 可通过 `graphql` 在远程发起 http 请求并执行监控工具，将产生的数据进行聚合后返回，用户可自定义运行时扩展插件进行在线数据分析；可外接时序数据库，如 `InfluxDB` 等，作为可选的信息持久化存储和数据分析方案；

除了收集容器中的一般系统运行时内核指标，例如系统调用、网络连接、文件访问、进程执行等，我们在探索实现过程中还发现目前对于 `lua` 和 `nginx` 相关用户态 `profile` 工具和指标可观测性开源工具存在一定的空白，但又有相当大的潜在需求；因此我们还计划添加一系列基于 uprobe 的用户态 `nginx/lua` 追踪器，作为可选的扩展方案；（这部分需求来自中科院开源之夏， APISIX 社区的选题）

和过去常用的 `BCC` 不同，`Eunomia` 基于 `Libbpf` + BPF CO-RE（一次编译，到处运行）开发。Libbpf 作为 BPF 程序加载器，接管了重定向、加载、验证等功能，BPF 程序开发者只需要关注 BPF 程序的正确性和性能即可。这种方式将开销降到了最低，且去除了庞大的依赖关系，使得整体开发流程更加顺畅。目前，我们已经发布了 `pre-release` 的版本，其中部分功能已经可以试用，只需下载二进制文件即可运行，


### 2.2. Tutorial

`Eunomia` 的 `ebpf` 追踪器部分是从 `libbpf-tools` 中得到了部分灵感，但是目前关于 ebpf 的资料还相对零散且过时，这也导致了我们在前期的开发过程中走了不少的弯路。因此, 我们也提供了一系列教程，以及丰富的参考资料，旨在降低新手学习eBPF技术的门槛，试图通过大量的例程解释、丰富对 `eBPF、libbpf、bcc` 等内核技术和容器相关原理的认知，让后来者能更深入地参与到 ebpf 的技术开发中来。另外，`Eunomia` 也可以被单独编译为 C++ 二进制库进行分发，可以很方便地添加自定义 libbpf检查器，或者直接利用已有的功能来对 syscall 等指标进行监测，教程中也会提供一部分 `EUNOMIA` 扩展开发接口教程。

> 教程目前还在完善中。

1. [eBPF介绍](doc/tutorial/0_eBPF介绍.md)
2. [eBPF开发工具介绍: BCC/Libbpf，以及其他](doc/tutorial/1_eBPF开发工具介绍.md)
3. [基于libbpf的内核级别跟踪和监控: syscall, process, files 和其他](doc/tutorial/2_基于libbpf的内核级别跟踪和监控.md)
4. [基于uprobe的用户态nginx相关指标监控](doc/tutorial/3_基于uprobe的用户态nginx相关指标监控.md)
5. [seccomp权限控制](doc/tutorial/4_seccomp权限控制.md)
6. [上手Eunomia: 基于Eunomia捕捉内核事件](doc/tutorial/x_基于Eunomia捕捉内核事件.md)

## 3. 比赛题目分析和相关资料调研

### 3.1. 题目描述

容器是一种应用层抽象，用于将代码和依赖资源打包在一起。多个容器可以在同一台机器上运行，共享操作系统内核。这使得容器的隔离性相对较弱，带来安全上的风险，最严重时会导致容器逃逸，严重影响底层基础设施的保密性、完整性和可用性。

eBPF 是一个通用执行引擎，能够高效地安全地执行基于系统事件的特定代码，可基于此开发性能分析工具**、网络数据包过滤、系统调用过滤，**系统观测和分析等诸多场景。eBPF可以由hook机制在系统调用被使用时触发，也可以通过kprobe或uprobe将eBPF程序附在内核/用户程序的任何地方。

这些机制让eBPF的跟踪技术可以有效地感知容器的各项行为，包括但不限于：

- 容器对文件的访问
- 容器对系统的调用
- 容器之间的互访

请基于eBPF技术开发一个监控工具，该工具可以监控容器的行为，并生成报表（如json文件）将各个容器的行为分别记录下来以供分析。

- **第一题：行为感知**

  编写eBPF程序，感知容器的各项行为。
- **第二题：信息存储**

  在第一题的基础上，令工具可以将采集到的数据以特定的格式保存在本地。
- **第三题：权限推荐（可选）**

  Seccomp是Linux内核的特性，开发者可以通过seccomp限制容器的行为。capabilities则将进程作为root的权限分成了各项更小的权限，方便			调控。这两个特性都有助于保障容器安全，但是因为业务执行的逻辑差异，准确配置权限最小集非常困难。请利用上面开发的监控工具，分析业务容器的行为记录报表，然后基于报表自动推荐精准的权限配置最小集。
### 3.2. 赛题分析

TODO

### 3.3. 相关资料调研

#### 3.3.1. ebpf

eBPF是一项革命性的技术，可以在Linux内核中运行沙盒程序，而无需更改内核源代码或加载内核模块。通过使Linux内核可编程，基础架构软件可以利用现有的层，从而使它们更加智能和功能丰富，而无需继续为系统增加额外的复杂性层。

* 优点：低开销

  eBPF 是一个非常轻量级的工具，用于监控使用 Linux 内核运行的任何东西。虽然 eBPF 程序位于内核中，但它不会更改任何源代码，这使其成为泄露监控数据和调试的绝佳伴侣。eBPF 擅长的是跨复杂系统实现无客户端监控。 
* 优点：安全

  解决内核观测行的一种方法是使用内核模块，它带来了大量的安全问题。而eBPF 程序不会改变内核，所以您可以保留代码级更改的访问管理规则。此外，eBPF 程序有一个验证阶段，该阶段通过大量程序约束防止资源被过度使用，保障了运行的ebpf程序不会在内核产生安全问题。
* 优点：精细监控、跟踪

  eBPF 程序能提供比其他方式更精准、更细粒度的细节和内核上下文的监控和跟踪标准。并且eBPF监控、跟踪到的数据可以很容易地导出到用户空间，并由可观测平台进行可视化。 
* 缺点：很新

  eBPF 仅在较新版本的 Linux 内核上可用，这对于在版本更新方面稍有滞后的组织来说可能是令人望而却步的。如果您没有运行 Linux 内核，那么 eBPF 根本不适合您。

#### 3.3.2. ebpf 开发工具技术选型

#### 3.3.3. 容器可观测性

#### 3.3.4. 信息可视化展示

#### 3.3.5. 容器运行时安全

确保容器运行时安全的关键点[1]：

- 使用 `ebpf` 跟踪技术自动生成容器访问控制权限。包括：容器对文件的可疑访问，容器对系统的可疑调用，容器之间的可疑互访，检测容器的异常进程，对可疑行为进行取证。例如：
- 检测容器运行时是否创建其他进程。
- 检测容器运行时是否存在文件系统读取和写入的异常行为，例如在运行的容器中安装了新软件包或者更新配置。
- 检测容器运行时是否打开了新的监听端口或者建立意外连接的异常网络活动。
- 检测容器中用户操作及可疑的 shell 脚本的执行。

## 4. 系统框架设计

### 4.1. 系统设计

### 4.2. 模块设计

### 4.3. 功能设计

### 4.4. ebpf 主要观测点

### 4.5. ebpf 可观测信息设计

### 4.6. 重要数据结构设计

### 4.7. 安全规则设计

## 5. 开发计划

### 5.1. 日程表

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

### 5.2. 未来的工作方向

## 6. 比赛过程中的重要进展

## 7. 系统测试情况

### 7.1. 快速上手


### 7.2. 命令行测试情况
        各项命令测试结果如下：
#### tracker系列命令




### 7.3. 容器测试情况


### 7.4. 信息可视化测试情况： prometheus and grafana


### 7.5. CI/持续集成

## 8. 遇到的主要问题和解决方法

### 8.1. 如何设计 ebpf 挂载点
        如何设计挂载点是ebpf程序在书写时首先需要考虑的问题。ebpf程序是事件驱动的，即只有系统中发生了我们预先规定的事件，我们的程序才会被调用。因此，ebpf挂载点的选择直接关系到程序能否在我们需要的场合下被启动。
        我们在选择挂载点时，首先需要明白的是我们需要在什么情况下触发处理函数，然后去寻找合适的挂载点。ebpf的挂载点有多种类型，较为常用的挂载点是`tracepoint`，`k/uprobe`，`lsm`等。
        `tracepoint`是一段静态的代码，以打桩的形式存在于程序源码中，并向外界提供钩子以挂载。一旦处理函数挂载到了钩子上，那么当钩子对应的事件发生时，处理函数就会被调用。由于`tracepoint`使用较为方便，且覆盖面广，ABI也较为稳定，他是我们设计挂载点的一个重要考虑对象。目前Linux已经有1000多个tracepoint可供选择，其支持的所有类型可以在`/sys/kernel/debug/tracing/events/`目录下看到，而至于涉及到的参数格式和返回形式，用户可以使用`cat`命令，查看对应`tracepoint`事件下的format文件得到。如下便是`sched_process_exec`事件的输出格式。
![](./imgs/report/tracepoint_example1.png)
        用户也可以直接访问`tracepoint`的源码获得更多信息。在Linux源码的`./include/trace/events`目录下，用户可以看到Linux中实现tracepoint的源码。  
        `k/uprobe`是Linux提供的，允许用户动态插桩的方式。由于`tracepoint`是静态的，如果用户临时需要对一些其不支持的函数进行追踪，就无法使用`tracepoint`，而`k/uprobe`允许用户事实对内核态/用户态中某条指令进行追踪。用户在指定了该指令的位置并启用`k/uprobe`后，当程序运行到该指令时，内核会自动跳转到我们处理代码，待处理完成后返回到原处。相较于`tracepoint`，`k/uprobe`更为灵活，如果你需要追踪的指令不被`tracepoint`所支持，可以考虑使用`k/uprobe`。  
         `lsm`是Linux内核安全模块的一套框架，其本质也是插桩。相较于`tracepoint`，`lsm`主要在内核安全的相关路径中插入了hook点。因此如果你希望你的代码检测一些和安全相关的内容，可以考虑使用`lsm`。其所有钩子的定义在Linux源码的`./include/linux/lsm_hook_defs.h`中，你可以从中选择初你所需要的hook点。


### 8.2. 如何进行内核态数据过滤和数据综合
        ebpf程序在内核态处理数据有诸多不便，有许多库我们都无法使用，如果遇上的复杂的数据过滤和数据综合，我们需要手动实现很多函数。因此我们认为更为合理的处理方案是将内核态的数据使用ebpf-map传输到用户态，在用户态进行过滤和综合，之后再输出。

### 8.3. 如何定位容器元信息

        `docker`有大量命令可以让我们看到容器的信息。比如`docker ps`命令就可以容器的ID，容器的名称等等。`docker insepect`命令则可以让我们看到更多容器信息。我们可以使用在程序中执行这些shell指令，捕获输出并解析后即可得到容器的元信息。

### 8.4. 如何设计支持可扩展性的数据结构

        首先尽可能降低各个模块的耦合性，这样在修改时可以较为方便地完成改动。其次，在最初设计时为未来可能的扩展预留位置。

## 9. 分工和协作



## 10. 提交仓库目录和文件描述


### 10.1. 项目仓库目录结构

          本仓库的主要目录结构如下所示：   

  ```
  ├─bpftools  
  │  ├─container  
  │  ├─files  
  │  ├─ipc  
  │  ├─process  
  │  ├─seccomp  
  │  ├─syscall  
  │  └─tcp  
  ├─cmake  
  ├─doc  
  │  ├─develop_doc   
  │  ├─imgs  
  │  └─tutorial  
  ├─include  
  │   └─eunomia  
  │       └─model  
  ├─libbpf  
  ├─src  
  ├─test  
  │   └─src  
  ├─third_party  
  │       └─prometheus-cpp  
  ├─tools  
  └─vmlinux  
  ```
### 10.2. 各目录及其文件描述
#### bpftools目录

          本目录内的所有文件均为基于ebpf开发的内核态监视代码，
共有7个子目录，子目录名表示了子目录内文件所实现的模块。比如process子目录代表了其中的文件
主要实现了进程追踪方面的ebpf内核态代码，其他子目录同理。

####  cmake目录

        本项目使用cmake进行编译，本目录中的所有文件都是本项目cmake
的相关配置文件。

#### doc目录

        本目录内的所有文件为与本项目相关的文档，其中develop_doc目录为开发
文档，其中记录了本项目开发的各种详细信息。tutorial目录为本项目为所有想进行ebpf开发的同学所设计的
教学文档，其中会提供一些入门教程，方便用户快速上手。imgs目录为开发文档和教学文档中所需要的一些
图片。

#### include目录

        本项目中用户态代码的头文件均会存放在本目录下。eunomia子目录中存放的
是各个模块和所需要的头文件，eunomia下的model子目录存放的是各个头文件中的一些必要结构体经过抽象后
的声明。

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

## 11. 比赛收获

### 郑昱笙同学


### 张典典同学

### 濮雯旭同学


## 12. 附录

### 12.1. Prometheus 观测指标

## Process Metrics

### Metrics List
| **Metric Name** | **Type** | **Description** |
| --- | --- | --- |
| `eunomia_observed_process_start` | Counter | Number of observed process start |
| `eunomia_observed_process_end` | Counter | Number of observed process end |

### Labels List
| **Label Name** | **Example** | **Notes** |
| --- | --- | --- |
| `node` | worker-1 | Node name represented in Kubernetes cluster |
| `pod` | default | Name of the pod |
| `mount_namespace` | 46289463245 | Mount Namespace of the pod |
| `container_name` | Ubuntu | The name of the container |
| `container_id` | 1a2b3c4d5e6f | The shorten container id which contains 12 characters |
| `pid` | 12344 | The pid of the running process |
| `comm` | ps | The command of the running process |
| `filename` | /usr/bin/ps | The exec file name |
| `exit_code` | 0 | The exit code |
| `duration_ms` | 375 | The running time |


## files Metrics

### Metrics List
| **Metric Name** | **Type** | **Description** |
| --- | --- | --- |
| `eunomia_observed_files_read_count` | Counter | Number of observed files read count |
| `eunomia_observed_files_write_count` | Counter | Number of observed files write count |
| `eunomia_observed_files_write_bytes` | Counter | Number of observed files read bytes |
| `eunomia_observed_files_read_bytes` | Counter | Number of observed files write bytes |

### Labels List
| **Label Name** | **Example** | **Notes** |
| --- | --- | --- |
| `comm` | eunomia | The command of the running process |
| `filename` | online | The exec file name |
| `pid` | 7686 | The pid of the running proces |
| `type` | 82 | Type of comm |

## Tcp Connect Metrics

### Metrics List
| **Metric Name** | **Type** | **Description** |
| --- | --- | --- |
| `eunomia_observed_tcp_v4_count` | Counter | Number of observed tcp v4 connect count |
| `eunomia_observed_tcp_v6_count` | Counter | Number of observed tcp v6 connect count |

### Labels List
| **Label Name** | **Example** | **Notes** |
| --- | --- | --- |
| `dst` | 127.0.0.1 | Destination of TCP connection |
| `pid` | 4036 | The pid of the running proces |
| `port` | 20513 | TCP exposed ports |
| `src` | 127.0.0.1 | Resources of TCP connection |
| `container_id` | 1a2b3c4d5e6f | The shorten container id which contains 12 characters |
| `task` | Socket Thread | The task of the running process |
| `uid` | 1000 | The uid of the running proces |


## Syscall Metrics

### Metrics List
| **Metric Name** | **Type** | **Description** |
| --- | --- | --- |
| `eunomia_observed_syscall_count` | Counter | Number of observed syscall count |

### Labels List
| **Label Name** | **Example** | **Notes** |
| --- | --- | --- |
| `comm` | firefox | The command of the running process |
| `pid` | 4036 | The pid of the running proces |
| `syscall` | writev | Name of the syscall called by running process |

## Security Event Metrics

### Metrics List
| **Metric Name** | **Type** | **Description** |
| --- | --- | --- |
| `eunomia_seccurity_warn_count` | Counter | Number of observed security warnings |
| `eunomia_seccurity_event_count` | Counter | Number of observed security event |
| `eunomia_seccurity_alert_count` | Counter | Number of observed security alert |

### Labels List
| **Label Name** | **Example** | **Notes** |
| --- | --- | --- |
| `comm` | firefox | The command of the running process |
| `pid` | 4036 | The pid of the running proces |
| `syscall` | writev | Name of the syscall called by running process |


## Service Metrics

Service metrics are generated from the eunomia server-side events, which are used to show the quality of eunomia own service.

### Metrics List
| **Metric Name** | **Type** | **Description** |
| --- | --- | --- |
| `eunomia_run_tracker_total` | Counter | Total number of running trackers |

### Labels List
| **Label Name** | **Example** | **Notes** |
| --- | --- | --- |
| `node` | worker-1 | Node name represented in Kubernetes cluster |
| `namespace` | default | Namespace of the pod |
| `container` | api-container | The name of the container |
| `container_id` | 1a2b3c4d5e6f | The shorten container id which contains 12 characters |
| `ip` | 10.1.11.23 | The IP address of the entity |
| `port` | 80 | The listening port of the entity |

## PromQL Example

Here are some examples of how to use these metrics in Prometheus, which can help you understand them faster.

| **Describe** | **PromQL** |
| --- | --- |
| Request counts | `sum(increase(eunomia_observed_tcp_v4_count{}[1m])) by(task)` |
| read rate | `sum(rate(eunomia_observed_files_read_bytes{}[1m])) by(comm)` |
| write rate | `sum(rate(eunomia_observed_files_write_count{}[1m])) by(comm)` |


### 12.2. 命令行工具帮助信息

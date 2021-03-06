# Eunomia

A lightweight container monitoring solution covering the entire life cycle based on eBPF 

[![Actions Status](https://github.com/filipdutescu/modern-cpp-template/workflows/MacOS/badge.svg)](https://github.com/filipdutescu/modern-cpp-template/actions)
[![Actions Status](https://github.com/filipdutescu/modern-cpp-template/workflows/Windows/badge.svg)](https://github.com/filipdutescu/modern-cpp-template/actions)
[![Actions Status](https://github.com/filipdutescu/modern-cpp-template/workflows/Ubuntu/badge.svg)](https://github.com/filipdutescu/modern-cpp-template/actions)
[![codecov](https://codecov.io/gh/filipdutescu/modern-cpp-template/branch/master/graph/badge.svg)](https://codecov.io/gh/filipdutescu/modern-cpp-template)
[![GitHub release (latest by date)](https://img.shields.io/github/v/release/filipdutescu/modern-cpp-template)](https://github.com/filipdutescu/modern-cpp-template/releases)

<!-- TOC -->

- [0.概述](#0概述)
- [1.项目介绍](#1项目介绍)
  - [项目描述](#项目描述)
  - [可能的检测方式](#可能的检测方式)
- [2.项目目标](#2项目目标)
- [3.开发计划](#3开发计划)
- [4.使用说明](#4使用说明)
    - [目录结构](#目录结构)
    - [安装教程](#安装教程)
    - [其他](#其他)

<!-- /TOC -->

# 0.概述

本项目由两部分组成：

* 一个零基础入门 `eBPF` 技术的教程实践和对应的命令行工具集，主要使用 `C/C++` 语言开发, 同时作为原型验证;
* 一个基于 `eBPF` 技术实现的用于监控容器的工具(**Eunomia**), 包含 `profile`、容器集群网络可视化分析、容器安全感知告警、一键部署、持久化存储监控等功能, 主要使用 Go 语言开发, 力求为工业界提供覆盖容器全生命周期的轻量级开源监控解决方案;

理论部分，目标旨在降低新手学习eBPF技术的门槛，试图通过大量的例程解释、丰富对eBPF、libbpf、bcc等内核技术的认知，该部分来自于学习实践过程中积累的各类学习资料、与开发者对eBPF技术逐步深化认知的过程。同时，结合本课题项目的来源，将实践部分拆分为X个实验Labs，配以详细丰富的踩坑经验。各Lab通过设置对操作系统不同主题的实验，进一步加深学生对操作系统中进程、线程、Tcp、文件系统等概念以及 `namespace` `、cgroup` 等内核机制的直观认知。

实践部分，来源于下述课题，主要是以X语言通过XX技术实现了一个用于监控容器行为的工具。开发过程符合软件工程开发规范，过程文档详细充实，测试用例丰富。每行代码均有相关注释。工具包含 `profile`、容器集群网络可视化分析、容器安全感知告警、一键部署、持久化存储监控等功能, 主要使用 Go 语言开发, 力求为工业界提供覆盖容器全生命周期的轻量级开源监控解决方案。

旨在：作为操作系统课程、 `eBPF` 开发入门、`docker`机制学习的补充材料, 

---

当前进度：

1. 与导师保持微信联系,定期联系线上会议
2. 与导师沟通,明确了项目目标
3. 完成了基本的功能性代码：
4. 构建了一个基于命令行程序的入口工具
5. 完成了整体的文档框架
6. 完成了多份文档：
[目标描述](https://gitlab.eduxiji.net/zhangdiandian/project788067-89436/-/blob/master/doc/develop_doc/1_目标描述.md)
[调研文档](https://gitlab.eduxiji.net/zhangdiandian/project788067-89436/-/blob/master/doc/develop_doc/2_调研文档.md)
[系统设计](https://gitlab.eduxiji.net/zhangdiandian/project788067-89436/-/blob/master/doc/develop_doc/3_系统设计.md)
[开发计划](https://gitlab.eduxiji.net/zhangdiandian/project788067-89436/-/blob/master/doc/develop_doc/4_开发计划.md)

TODO:

1. 继续丰富行为感知的功能性代码：
2. 添加监控信息的可视化功能
3. 补充、完善开发文档
4. 编写ebpf教程与Labs

# 1.项目介绍

> 项目来源：[eBPF-based-monitor-for-container](https://github.com/oscomp/proj118-eBPF-based-monitor-for-container)

## 项目描述

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

## 可能的检测方式

确保容器运行时安全的关键点[1]：

- 使用 `ebpf` 跟踪技术自动生成容器访问控制权限。包括：容器对文件的可疑访问，容器对系统的可疑调用，容器之间的可疑互访，检测容器的异常进程，对可疑行为进行取证。例如：
- 检测容器运行时是否创建其他进程。
- 检测容器运行时是否存在文件系统读取和写入的异常行为，例如在运行的容器中安装了新软件包或者更新配置。
- 检测容器运行时是否打开了新的监听端口或者建立意外连接的异常网络活动。
- 检测容器中用户操作及可疑的 shell 脚本的执行。

# 2.项目目标

1. 实现一个接近工业界的工具
2. 输出一系列开发文档

* 开发工具的使用教程——环境搭建；工具说明书；
* 开发工具的设计文档——设计架构、各模块详细设计、数据流程；
* 开发工具的开发文档——各模块的作用、函数的注释；
* 测试文档——测试环境的搭建教程；设计多种容器环境，进行测试；
* 过程文档——时间段、分工、迭代版次&版本变化；

3. 一份 `ebpf` 技术的入门教程与lab

# 3.开发计划

阶段一：学习ebpf相关技术栈（3.10~4.2）

* [X] 入门ebpf技术栈
* [X] 调研、学习 `bcc`
* [X] 调研、学习 `libbpf` 、`libbpf-bootstrap`
* [X] 调研、学习 `seccomp`
* [X] 输出调研文档

阶段二：项目设计（4.3~4.10）

* [X] 与mentor讨论项目需求、并设计功能模块
* [ ] 输出系统设计文档
* [ ] 输出模块设计文档

阶段三：开发迭代（4.10~6.1）

* [X] 实现进程信息监控（pid、ppid等）
* [X] 实现系统调用信息监控
* [X] 实现进程间通信监控
* [X] 实现tcp（ipv4、ipv6）通信监控
* [X] 实现监控信息存储功能（csv或json格式）
* [X] 完成了系统的原型验证功能
* [ ] 基于上述功能，实现命令行调用，完成版本v0.1
* [ ] 输出开发v0.1日志文档
* [ ] 实现进程id与容器id映射，进程信息过滤
* [ ] 添加“seccomp”功能
* [ ] 基于上述新增功能，迭代版本v0.2
* [ ] 输出开发v0.2日志文档
* [ ] 添加可视化模块
* [ ] 基于上述新增功能，迭代版本v0.3
* [ ] 输出开发v0.3日志文档
* [ ] 后续更新迭代

阶段四：开发测试（6.2~6.16）

* [ ] 设计测试场景（分别针对基础功能、权限控制、安全逃逸场景）
* [ ] 搭建测试环境
* [ ] 测试-开发
* [ ] 输出测试文档

阶段五：项目文档完善（6.17~7.1）

* [ ] 完善开发文档
* [ ] 完善教程文档
* [ ] 完善labs

# 4.使用说明

### 目录结构

* 项目的目录结构如下图所示

```
├── doc
│   ├── contribution.md
│   ├── Develop-Doc
│   └── tutorial
├── src
│   ├── files
│   ├── include
│   ├── process
│   │   ├── bootstrap.bpf.c
│   │   ├── bootstrap.c
│   │   ├── bootstrap.h
│   │   └── Makefile
│   ├── syscall
│   ├── ipc
│   └── tcp
├── libbpf
├── LICENSE
├── README.md
├── tools
├── utils
└── vmlinuxi
```

目录说明

* doc：
  * Develop-Doc：开发文档
  * Labs：教程+Lab系列
  * contribution.md：博客、社区贡献
* src：
  * process：监控进程信息
  * syscall：监控系统调用
  * files: 监控文件读取写入
  * ipc：监控进程间通信
  * tcp：抓取tcp通信两端的信息
* libbpf：
* tools：
* utils：
* vmlinux：
* README.md

### 安装教程

**Build**

Makefile build:

```sh
git submodule update --init --recursive       # check out libbpf
make install
```

For example, run process:

```sh
cd src/process
sudo ./process
```

### 其他

**成员**

指导老师：程泽睿志（华为）李东昂（浙江大学）

学生：郑昱笙，濮雯旭，张典典

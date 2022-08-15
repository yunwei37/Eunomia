# peformance and benchmarking

<!-- TOC -->

- [performance](#performance)
  - [基于 ebpf](#基于-ebpf)
  - [基于 libbpf](#基于-libbpf)
  - [C/C++](#cc)
  - [对 libbpf-tools 进行的优化：添加选项进行聚合](#对-libbpf-tools-进行的优化添加选项进行聚合)
  - [在内核态和用户态中采用环形缓冲器进行通信](#在内核态和用户态中采用环形缓冲器进行通信)
  - [在内核态进行 syscall 规则过滤](#在内核态进行-syscall-规则过滤)
- [benchmark](#benchmark)

<!-- /TOC -->

## performance

### 基于 ebpf

- 得益于 ebpf 的可编程特性，Eunomia 直接在内核中使用 eBPF 执行过滤、聚合、度量统计和直方图收集，避免向用户空间 agent 发送大量低信号事件，大大减少了系统的开销。

### 基于 libbpf

- 得益于 Libbpf + BPF CO-RE（一次编译，到处运行）的强大性能，Eunomia 仅需安装一个 agent 就可以收集这台主机相关的系统数据，最小仅需约 4MB 即可在支持的内核上或容器中启动跟踪，启动一个跟踪器仅需约 100 ms

### C/C++

使用了 C/C++ 高效的数据结构和多线程/多进程分析处理，以提供高效和快速的数据收集手段，在大多数使用情况下仅使用 2-3% 的 CPU 和少量内存，开启基础的6个tracker内存占用仅50MB；

### 对 libbpf-tools 进行的优化：添加选项进行聚合

- 例如，我们对 files、 tcpconnect、syscall 等工具，都进行了更进一步的优化，在内核中根据时间间隔、次数来聚合，避免频繁上报事件；
  - 根据次数进行统计，如 syscall 统计每个进程调用 syscall 的次数并存储在map中，而不是直接上报；
  - 根据 pid、namespace、cgroup 进行过滤；
  - process 短于一定时间间隔的短进程不予统计；
  - 根据一定时间进行统计采样，如 files
- 尽可能在内核中根据容器 cgroups、pid、uid 等 target ，提供丰富的选项进行过滤；

### 在内核态和用户态中采用环形缓冲器进行通信

- 在内核和用户态之间使用 map 和环形缓冲区进行通信，避免流量突增造成流水线堵塞；多余的事件会被直接丢弃；

### 在内核态进行 syscall 规则过滤

对于一部分的安全规则，例如某些特定的系统调用，我们可以再启动检查器的时候往内核中添加过滤器或触发器，当规则匹配的时候才进行上报处理，避免用户态的频繁处理事件；

## benchmark

> 虚拟机环境：Linux ubuntu 5.13.0-44-generic #49~20.04.1-Ubuntu SMP x86_64 GNU/Linux 6 核，12 GB 内存；



使用 top 查看 eunomia 的内存和cpu占用情况

![top](imgs/top2.png)

使用 openresty 和 APISIX 在本机上启动一个包含6个容器和负载均衡的网络服务，以及 Prometheus 和 Grafana ，使用 wrk 进行压力测试：

```
Linux ubuntu 5.13.0-44-generic #49~20.04.1-Ubuntu SMP x86_64 GNU/Linux
4 核，12 GB 内存：
```

这是未开启 eunomia server 的情况：

![no](imgs/openresty_no_eunomia.png)

这是启动 eunomia server 后的情况，使用默认配置并启用 process/container、files、tcp 等4-5个主要的跟踪器，在同样环境下进行测试：

![no](imgs/openresty_with_eunomia.png)

可以看到大约仅有 4% 的性能损失；

> OpenResty® 是一个基于 Nginx 与 Lua 的高性能 Web 平台，其内部集成了大量精良的 Lua 库、第三方模块以及大多数的依赖项。用于方便地搭建能够处理超高并发、扩展性极高的动态 Web 应用、Web 服务和动态网关。web开发人员可以使用lua编程语言，对核心以及各种c模块进行编程，可以利用openresty快速搭建超1万并发高性能web应用系统。这里的 benchmark 参考了：https://openresty.org/en/benchmark.html



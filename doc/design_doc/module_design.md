# 模块设计


## tracker_manager

  负责启动和停止 ebpf collector，并且和 ebpf collector 通信（每个 tracer 是一个线程）；

- start tracker
- stop tracker(remove tracker)

Currently we have 5 main trackers:

- process
- syscall
- tcp
- files
- ipc

## container_manager

  负责观察 container 的启动和停止，在内存中保存每个 container 的相关信息：（cgroup，namespace），同时负责 container id, container name 等 container mata 信息到 pid 的转换（提供查询接口）

## seccomp_manager

  负责对 process 进行 seccomp 限制

## data_collector

  收集数据，再决定怎么办；传给 database 还是聚合还是交给别的地方还是打印

- collect_string
- collect_json
- collet_object

## container detection

容器安全检测规则引擎，可以帮助您检测事件流中的可疑行为模式。

## security analyzer

安全分析模块，通过ebpf采集到的底层相关数据，运用包括AI在内的多种方法进行安全性分析。

## prometheus exporter

将数据导出成Prometheus需要的格式，在Prometheus中保存时序数据，方便后续持久化和可视化功能。

## config loader

   解析 toml

## cmd

   命令行解析模块，将命令行字符串解析成对应的参数选项，对Eunomia进行配置。

## core

    负责装配所需要的 tracker，配置对应的功能用例，并且启动系统。

## server

   http 通信：通过 `graphql` 在远程发起 http 请求并执行监控工具，将产生的数据进行聚合后返回，用户可自定义运行时扩展插件进行在线数据分析；

## Eunomia


`Eunomia` 是一个使用 C/C++ 开发的基于eBPF的云原生监控工具，旨在帮助用户了解容器的各项行为、监控可疑的容器安全事件，力求为工业界提供覆盖容器全生命周期的轻量级开源监控解决方案。它使用 `Linux` `eBPF` 技术在运行时跟踪您的系统和应用程序，并分析收集的事件以检测可疑的行为模式。目前，它包含 `profile`、容器集群网络可视化分析*、容器安全感知告警、一键部署、持久化存储监控等功能。

* [X] 开箱即用：以单一二进制文件或 `docker` 镜像方式分发，一次编译，到处运行，一行代码即可启动，包含多种 ebpf 工具和多种监测点，支持多种输出格式（json, csv, etc)；
* [X] 可集成 `prometheus` 和 `Grafana`，作为监控可视化和预警平台；
* [X] 可自定义运行时安全预警规则，也可以自动收集进程系统调用行为并通过 seccomp 进行限制；
* [ ] 可外接时序数据库，如 `InfluxDB` 等，作为可选的信息持久化存储方案；
* [ ] 可通过 `graphql` 在远程发起 http 请求并执行监控工具，将产生的数据进行聚合后返回，用户可自定义运行时扩展插件进行在线数据分析；

除了收集容器中的一般系统运行时内核指标，例如系统调用、网络连接、文件访问、进程执行等，我们在探索实现过程中还发现目前对于 `lua` 和 `nginx` 相关用户态 `profile` 工具和指标可观测性开源工具存在一定的空白，但又有相当大的潜在需求；因此我们还计划添加一系列基于 uprobe 的用户态 `nginx/lua` 追踪器，作为可选的扩展方案；

和过去常用的 `BCC` 不同，`Eunomia` 基于 `Libbpf` + BPF CO-RE（一次编译，到处运行）开发。Libbpf 作为 BPF 程序加载器，接管了重定向、加载、验证等功能，BPF 程序开发者只需要关注 BPF 程序的正确性和性能即可。这种方式将开销降到了最低，且去除了庞大的依赖关系，使得整体开发流程更加顺畅。

## 命令行工具

### 功能：

（cmd需要换成我们的最后的命名

1. 直接运行单一的 ebpf 工具，例如：

    ```
    ./cmd run tcpconnect [-c container_id] [-p PID] [-T second] 
    ```

    指定 container id 或容器名称或 pid 或 namespace，即可监控该容器下的 tcp 连接事件；

    ```
    ./cmd run tcpconnect [--fmt json]
    ```

    监控所有的 tcp 连接事件，并且以 json 格式输出

    每个子功能的参数可以自定义，参考 libbpf-tools

    除了 tcp 以外，还可以有：

    - syscall
    - ipc
    - file w/r

    还没做的：

    - perf event
    - （参考 libbpf-tools

2. 作为守护进程运行，记录日志（直接打印或者输出到某个文件

    ```
    ./cmd daemon [-c container id] [-p PID]
    ```

    监控某一个 docker id 或 process 的相关行为；

    自动对高危行为进行预警：比如调用了某些危险 syscall，读写了某些危险文件（这里的高危行为会硬编码在代码里，属于确定的高危行为，基本上会产生 docker 逃逸那种）

    ```
    ./cmd daemon [-c container id] [--config config.toml]
    ```

    通过 toml 用户定义可能的危险行为（syscall，文件读写），并产生日志预警；并且自定义需要追踪的部分，和时序数据库连接

    toml example：



3. seccomp

    ```
    ./cmd daemon [-c container_id ] [-p PID] [--seccomp syscall_id_file] 
    ```

    加载配置文件，对 相应 pid 和 container 进行限制；

    ```
    ./cmd seccomp [-p PID] [-T 30] [-o file]
    ```

    对某个正常的程序追踪 30s 产生 syscall 表，用来给 seccomp 进行限制；

    ```
    ./cmd seccomp --source -sc [container id] -sp [PID] 
    --target -tc [container id] -tp [PID] 
    ```

    追踪某个正常的程序，并且把它的 syscall 通过 seccomp 应用限制到其他的可能被污染的 process 上面；

4. server

    ```
    ./cmd server [--prometheus] [--config file ...]
    ```

    作为 prometheus exporter 运行守护进程；

    ```
    ./cmd server [--listen 8080]
    ```

    在某个端口监听 http 控制请求，可以和我们自己的前端进行通信，，并且通过前端进行简单控制和查看网络连接图表（简单画几个按钮和图就好，一页就行？）；
# ebpf docker

## describe

容器是一种应用层抽象，用于将代码和依赖资源打包在一起。多个容器可以在同一台机器上运行，共享操作系统内核。这使得容器的隔离性相对较弱，带来安全上的风险，最严重时会导致容器逃逸，严重影响底层基础设施的保密性、完整性和可用性。

eBPF 是一个通用执行引擎，能够高效地安全地执行基于系统事件的特定代码，可基于此开发性能分析工具**、网络数据包过滤、系统调用过滤，**系统观测和分析等诸多场景。eBPF可以由hook机制在系统调用被使用时触发，也可以通过kprobe或uprobe将eBPF程序附在内核/用户程序的任何地方。

这些机制让eBPF的跟踪技术可以有效地感知容器的各项行为，包括但不限于：

- 容器对文件的访问
- 容器对系统的调用
- 容器之间的互访

请基于eBPF技术开发一个监控工具，该工具可以监控容器的行为，并生成报表（如json文件）将各个容器的行为分别记录下来以供分析。

第一题：行为感知

编写eBPF程序，感知容器的各项行为。

第二题：信息存储

在第一题的基础上，令工具可以将采集到的数据以特定的格式保存在本地。

（可选）第三题：权限推荐

Seccomp是Linux内核的特性，开发者可以通过seccomp限制容器的行为。capabilities则将进程作为root的权限分成了各项更小的权限，方便调控。这两个特性都有助于保障容器安全，但是因为业务执行的逻辑差异，准确配置权限最小集非常困难。请利用上面开发的监控工具，分析业务容器的行为记录报表，然后基于报表自动推荐精准的权限配置最小集。

## 可能的检测方式

确保容器运行时安全的关键点[1]：

- 使用 ebpf 跟踪技术自动生成容器访问控制权限。包括：容器对文件的可疑访问，容器对系统的可疑调用，容器之间的可疑互访，检测容器的异常进程，对可疑行为进行取证。例如：

- 检测容器运行时是否创建其他进程。
- 检测容器运行时是否存在文件系统读取和写入的异常行为，例如在运行的容器中安装了新软件包或者更新配置。
- 检测容器运行时是否打开了新的监听端口或者建立意外连接的异常网络活动。
- 检测容器中用户操作及可疑的 shell 脚本的执行。

## docker 原理

### Cgroup

#### Cgroup介绍

CGroup 是 Control Groups 的缩写，是 Linux 内核提供的一种可以限制、记录、隔离进程组 (process groups) 所使用的物力资源 (如 cpu memory i/o 等等) 的机制。2007 年进入 Linux 2.6.24 内核，CGroups 不是全新创造的，它将进程管理从 cpuset 中剥离出来，作者是 Google 的 Paul Menage。CGroups 也是 LXC 为实现虚拟化所使用的资源管理手段。

#### CGroup 功能及组成

CGroup 是将任意进程进行分组化管理的 Linux 内核功能。CGroup 本身是提供将进程进行分组化管理的功能和接口的基础结构，I/O 或内存的分配控制等具体的资源管理功能是通过这个功能来实现的。这些具体的资源管理功能称为 CGroup 子系统或控制器。CGroup 子系统有控制内存的 Memory 控制器、控制进程调度的 CPU 控制器等。运行中的内核可以使用的 Cgroup 子系统由/proc/cgroup 来确认。

CGroup 提供了一个 CGroup 虚拟文件系统，作为进行分组管理和各子系统设置的用户接口。要使用 CGroup，必须挂载 CGroup 文件系统。这时通过挂载选项指定使用哪个子系统。

cgroups task_struct reference:

https://www.infoq.cn/article/docker-kernel-knowledge-cgroups-resource-isolation/

https://blog.csdn.net/punk_lover/article/details/78376430

> cgroup 指针指向了一个 cgroup 结构，也就是进程属于的 cgroup。进程受到子系统的控制，实际上是通过加入到特定的 cgroup 实现的，因为 cgroup 在特定的层级上，而子系统又是附和到上面的。通过以上三个结构，进程就可以和 cgroup 连接起来了：task_struct->css_set->cgroup_subsys_state->cgroup。

```c
static void fill_container_id(char *container_id) {
  struct task_struct *curr_task;
  struct css_set *css;
  struct cgroup_subsys_state *sbs;
  struct cgroup *cg;
  struct kernfs_node *knode, *pknode;
 
  curr_task = (struct task_struct *) bpf_get_current_task();
  css = curr_task->cgroups;
  bpf_probe_read(&sbs, sizeof(void *), &css->subsys[0]);
  bpf_probe_read(&cg,  sizeof(void *), &sbs->cgroup);
 
  bpf_probe_read(&knode, sizeof(void *), &cg->kn);
  bpf_probe_read(&pknode, sizeof(void *), &knode->parent);
 
  if(pknode != NULL) {
    char *aus;
 
    bpf_probe_read(&aus, sizeof(void *), &knode->name);
    bpf_probe_read_str(container_id, CONTAINER_ID_LEN, aus);
  }
}
```

## build

Makefile build:

```shell

$ git submodule update --init --recursive       # check out libbpf
$ cd examples/c
$ make
$ sudo ./bootstrap

```
# reference 

## ebpf

1. 基于 eBPF 实现容器运行时安全

    https://mp.weixin.qq.com/s/UiR8rjTt2SgJo5zs8n5Sqg

2. 基于ebpf统计docker容器网络流量

    https://blog.csdn.net/qq_32740107/article/details/110224623

3. BumbleBee: Build, Ship, Run eBPF tools

    https://www.solo.io/blog/solo-announces-bumblebee/

4. Container traffic visibility library based on eBPF

    https://github.com/ntop/libebpfflow

5. about libbpf

    https://nakryiko.com/posts/libbpf-bootstrap/#why-libbpf-bootstrap
    https://nakryiko.com/posts/bpf-core-reference-guide/

6. bcc to libbpf

    https://nakryiko.com/posts/bcc-to-libbpf-howto-guide/#setting-up-user-space-parts

6. good intro for trace point and kprobe in ebpf

    https://www.iserica.com/posts/brief-intro-for-tracepoint/
    https://www.iserica.com/posts/brief-intro-for-kprobe/

7. other

    https://lockc-project.github.io/book/index.html
    https://github.com/willfindlay/bpfcontain-rs

8. user space uprobe

    https://www.collabora.com/news-and-blog/blog/2019/05/14/an-ebpf-overview-part-5-tracing-user-processes/

9. ebpf secomp

    https://developers.redhat.com/articles/2021/12/16/secure-your-kubernetes-deployments-ebpf#how_does_the_bpf_recorder_work_

    https://github.com/kubernetes-sigs/security-profiles-operator/blob/main/internal/pkg/daemon/bpfrecorder/bpf/recorder.bpf.c
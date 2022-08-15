
# ebpf 主要观测点

- process追踪模块

  进程的追踪模块本项目主要设置了两个 `tracepoint` 挂载点。
  第一个挂载点形式为

  ```c
          SEC("tp/sched/sched_process_exec")
          int handle_exec(struct trace_event_raw_sched_process_exec *ctx)
          {
      
          }
  ```

  当进程被执行时，该函数会被调用，函数体中会从传入的上下文内容提取内容，我们需要的信息记录在Map中。
  第二个挂载点形式为

  ```c
          SEC("tp/sched/sched_process_exit")
          int handle_exit(struct trace_event_raw_sched_process_template *ctx)
          {
              
          }
  ```

  当有进程退出时，该函数会被调用，函数体同样会从传入的上下文内容提取内容，我们需要的信息记录在Map中。

- syscall追踪模块

  对于系统调用的追踪模块设置了一个 `tracepoint` 挂载点。挂载点形式为
  ```c
          SEC("tracepoint/raw_syscalls/sys_enter")
          int sys_enter(struct trace_event_raw_sys_enter *args)
          {
      
          }
  ```
  当有syscall发生时，其经过`sys_enter`执行点时我们的函数将会被调用，将相关信息存入map后供用户态读取。

- file追踪模块

  对于文件系统，我们设置了两个 `kprobe` 挂载点。第一个挂载点形式为
  ```c
          SEC("kprobe/vfs_read")
          int BPF_KPROBE(vfs_read_entry, struct file *file, char *buf, size_t count, loff_t *pos)
          {
      
          }
  ```
  第二个挂载点形式为
  ```c
          SEC("kprobe/vfs_write")
          int BPF_KPROBE(vfs_write_entry, struct file *file, const char *buf, size_t count, loff_t *pos)
          {
      
          }
  ```
  当系统中发生了文件读或写时，这两个执行点下的函数会被触发，记录相应信息。

-  tcp追踪模块   

    ```c
            SEC("kprobe/tcp_v6_connect")
            int BPF_KPROBE(tcp_v6_connect, struct sock *sk) {
              
            }

            SEC("kretprobe/tcp_v6_connect")
            int BPF_KRETPROBE(tcp_v6_connect_ret, int ret) {
              return exit_tcp_connect(ctx, ret, 6);
            }
    ```

- 部分集成工具追踪模块
  - oomkill  
  在此模块我们使用了kprobe接口，当发生进程因为内存耗尽而死亡的情况时会经过此路径。
    ```c
    SEC("kprobe/oom_kill_process")
    int BPF_KPROBE(oom_kill_process, struct oom_control *oc, const char *message)
    {
      /**
       * function body
      */
    }
    ```
  - bindsnoop   
  在此模块我们使用了kprobe接口，当系统中发生了端口绑定操作时内核便会经过此路径触发执行函数。
    ```c
    SEC("kprobe/inet_bind")
    int BPF_KPROBE(ipv4_bind_entry, struct socket *socket)
    {
      /**
       * function body
      */
    }
    SEC("kretprobe/inet_bind")

    int BPF_KRETPROBE(ipv4_bind_exit)
    {
      /**
       * function body
      */
    }

    SEC("tracepoint/syscalls/sys_enter_tkill")
    int tkill_entry(struct trace_event_raw_sys_enter *ctx)
    {
      /**
       * function body
      */
    }

    SEC("tracepoint/syscalls/sys_exit_tkill")
    int tkill_exit(struct trace_event_raw_sys_exit *ctx)
    {
      /**
       * function body
      */
    }

    SEC("tracepoint/syscalls/sys_exit_tgkill")
    int tgkill_exit(struct trace_event_raw_sys_exit *ctx)
    {
      /**
       * function body
      */
    }
    ```
  - biolatency   
  biolatency 设置了三个tracepoint, 当I/O事件发生时，会经过下面两个挂载点。
    ```c
    SEC("tracepoint/raw_syscalls/sys_enter")
    int sys_enter(struct trace_event_raw_sys_enter *args)
    {
      /**
       * function body
      */
    }

    SEC("tracepoint/raw_syscalls/sys_exit")
    int sys_exit(struct trace_event_raw_sys_exit *args)
    {
      /**
       * function body
      */
    }
    ```
    当I/O事件完成时，则会经过下面的挂载点。
    ```c
    SEC("tp_btf/block_rq_complete")
    int BPF_PROG(block_rq_complete, struct request *rq, int error,
      unsigned int nr_bytes)
    {
      /**
       * function body
      */
    }
    ```
  - biopattern   
  biopattern 与biolatency 类似，都为I/O事件有关的追踪工具，其挂载点也类似。biopattern 的挂载点处于I/O操作完成时。
    ```c
    SEC("tracepoint/block/block_rq_complete")
    int handle__block_rq_complete(struct trace_event_raw_block_rq_complete *ctx)
    {
      /**
       * function body
      */
    }
    ```
  - biosnoop   
  biosnoop也是跟踪磁盘I/O操作的工具，其挂载点与上文中的biolatency, biopattern类似。相较于这两个工具，其多了fentry和kprobe下的额外两个挂载点。
    ```c
    SEC("fentry/blk_account_io_start")
    int BPF_PROG(blk_account_io_start, struct request *rq)
    {
      return trace_pid(rq);
    }
    SEC("kprobe/blk_account_io_merge_bio")
    int BPF_KPROBE(blk_account_io_merge_bio, struct request *rq)
    {
      return trace_pid(rq);
    }
    ```
  - biostacks   
  因为某些I/O操作不是应用发起的，所以为了追踪这些I/O操作，biostacks在fentry和kprobe下挂载了执行函数。
    ```c
    SEC("fentry/blk_account_io_start")
    int BPF_PROG(blk_account_io_start, struct request *rq)
    {
      return trace_start(ctx, rq, false);
    }

    SEC("kprobe/blk_account_io_merge_bio")
    int BPF_KPROBE(blk_account_io_merge_bio, struct request *rq)
    {
      return trace_start(ctx, rq, true);
    }
    SEC("fentry/blk_account_io_done")
    int BPF_PROG(blk_account_io_done, struct request *rq)
    {
    /**
     * function body
    */
    }
    ```
  - bitesize     
  bitesize也是对I/O操作进行追踪的工具，其挂载点为
    ```c
    SEC("tp_btf/block_rq_issue")
    int BPF_PROG(block_rq_issue)
    {
      if (LINUX_KERNEL_VERSION >= KERNEL_VERSION(5, 11, 0))
        return trace_rq_issue((void *)ctx[0]);
      else
        return trace_rq_issue((void *)ctx[1]);
    }
    ```
  - capable   
  capable是与linux中capability机制相关的追踪工具，其使用kprobe中的挂载点
    ```c
    SEC("kprobe/cap_capable")
    int BPF_KPROBE(kprobe__cap_capable, const struct cred *cred, struct user_namespace *targ_ns, int cap, int cap_opt)
    {
      /**
       * function body
      */
    }
    ```
  - funclatency   
  funclatency是统计函数执行耗时的工具，其会挂载在目标函数进入前和完成后，统计函数执行耗时。其挂载点需要在用户态代码中进行注册绑定。
    ```c
    SEC("kprobe/dummy_kprobe")
    int BPF_KPROBE(dummy_kprobe)
    {
      /**
       * function body
      */
    }

    SEC("kretprobe/dummy_kretprobe")
    int BPF_KRETPROBE(dummy_kretprobe)
    {
      /**
       * function body
      */
    }
    ```
  - memleak   
  memleak是用来追踪用户分配内存和回收内存的工具，其挂载点为各个和内存分配相关联的函数。
    ```c
    SEC("uprobe/malloc")

    SEC("uretprobe/malloc")

    SEC("uprobe/calloc")

    SEC("uretprobe/calloc")

    SEC("uprobe/realloc")

    SEC("uretprobe/realloc")

    SEC("uprobe/memalign")

    SEC("uretprobe/memalign")

    SEC("uprobe/posix_memalign")

    SEC("uretprobe/posix_memalign")

    SEC("uprobe/valloc")

    SEC("uretprobe/valloc")

    SEC("uprobe/pvalloc")

    SEC("uretprobe/pvalloc")

    SEC("uprobe/aligned_alloc")

    SEC("uretprobe/aligned_alloc")

    SEC("uprobe/free")

    SEC("tracepoint/kmem/kmalloc")

    SEC("tracepoint/kmem/kfree")


    SEC("tracepoint/kmem/kmalloc_node")

    SEC("tracepoint/kmem/kmem_cache_alloc")

    SEC("tracepoint/kmem/kmem_cache_alloc_node")

    SEC("tracepoint/kmem/kmem_cache_free")

    SEC("tracepoint/kmem/mm_page_alloc")

    SEC("tracepoint/kmem/mm_page_free")

    SEC("tracepoint/percpu/percpu_alloc_percpu")

    SEC("tracepoint/percpu/percpu_free_percpu")
    ```
  - llcstat   
  llcstat是统计cache miss和cache reference信息的工具。其引入了linux中的perf_event工具，挂载点也在perf_event下。
    ```c
    SEC("perf_event")
    int on_cache_miss(struct bpf_perf_event_data *ctx)
    {
      return trace_event(ctx->sample_period, true);
    }

    SEC("perf_event")
    int on_cache_ref(struct bpf_perf_event_data *ctx)
    {
      return trace_event(ctx->sample_period, false);
    }
    ```
  - mountsnoop   
  mountsnoop是追踪mount和umount操作的工具，其挂载点为对应的tracepoint。
    ```c
    SEC("tracepoint/syscalls/sys_enter_mount")
    int mount_entry(struct trace_event_raw_sys_enter *ctx)
    {
      /**
       * function body
      */
    }

    SEC("tracepoint/syscalls/sys_exit_mount")
    int mount_exit(struct trace_event_raw_sys_exit *ctx)
    {
      /**
       * function body
      */
    }

    SEC("tracepoint/syscalls/sys_enter_umount")
    int umount_entry(struct trace_event_raw_sys_enter *ctx)
    {
      /**
       * function body
      */
    }

    SEC("tracepoint/syscalls/sys_exit_umount")
    int umount_exit(struct trace_event_raw_sys_exit *ctx)
    {
      /**
       * function body
      */
    }
    ```
  - opensnoop    
  opensnoop为追踪系统中open相关系统调用的工具，其使用对应的tracepoint作为挂载点。
    ```c
    SEC("tracepoint/syscalls/sys_enter_open")
    int tracepoint__syscalls__sys_enter_open(struct trace_event_raw_sys_enter* ctx)
    {
      /**
       * function body
      */
    }

    SEC("tracepoint/syscalls/sys_enter_openat")
    int tracepoint__syscalls__sys_enter_openat(struct trace_event_raw_sys_enter* ctx)
    {
      /**
       * function body
      */
    }
    SEC("tracepoint/syscalls/sys_exit_open")
    int tracepoint__syscalls__sys_exit_open(struct trace_event_raw_sys_exit* ctx)
    {
      /**
       * function body
      */
    }

    SEC("tracepoint/syscalls/sys_exit_openat")
    int tracepoint__syscalls__sys_exit_openat(struct trace_event_raw_sys_exit* ctx)
    {
      /**
       * function body
      */
    }
    ```
  - profile   
  profile是用于追踪程序执行调用流程的工具，其挂载点也是利用了perf_event。
    ```c
    SEC("perf_event")
    int do_perf_event(struct bpf_perf_event_data *ctx)
    {
      /**
       * function body
      */   
    }
    ```
  - sigsnoop  
  sigsnoop是监控系统中kill系统调用的工具，其挂载点为对应的tracepoint。
    ```c
    SEC("tracepoint/syscalls/sys_enter_kill")
    int kill_entry(struct trace_event_raw_sys_enter *ctx)
    {
      /**
       * function body
      */  
    }

    SEC("tracepoint/syscalls/sys_exit_kill")
    int kill_exit(struct trace_event_raw_sys_exit *ctx)
    {
      /**
       * function body
      */  
    }

    SEC("tracepoint/syscalls/sys_enter_tkill")
    int tkill_entry(struct trace_event_raw_sys_enter *ctx)
    {
      /**
       * function body
      */  
    }

    SEC("tracepoint/syscalls/sys_exit_tkill")
    int tkill_exit(struct trace_event_raw_sys_exit *ctx)
    {
      /**
       * function body
      */  
    }

    SEC("tracepoint/syscalls/sys_enter_tgkill")
    int tgkill_entry(struct trace_event_raw_sys_enter *ctx)
    {
      /**
       * function body
      */  
    }

    SEC("tracepoint/syscalls/sys_exit_tgkill")
    int tgkill_exit(struct trace_event_raw_sys_exit *ctx)
    {
      /**
       * function body
      */  
    }

    SEC("tracepoint/signal/signal_generate")
    int sig_trace(struct trace_event_raw_signal_generate *ctx)
    {
      /**
       * function body
      */  
    }  
    ```
  - syscount    
  syscount是统计系统或某个进程中发生各类syscall的总数或耗时的工具。其挂载点为tracepoint下syscall的入口和出口。
    ```c
    SEC("tracepoint/raw_syscalls/sys_enter")
    int sys_enter(struct trace_event_raw_sys_enter *args)
    {
      /**
       * function body
      */  
    }

    SEC("tracepoint/raw_syscalls/sys_exit")
    int sys_exit(struct trace_event_raw_sys_exit *args)
    {
      /**
       * function body
      */  
    }

    ```
  - tcpconnlat   
  tcpconnlat是检测tcp连接时延的工具，其挂载点为kprobe下tcp的一系列挂载点。
    ```c
    SEC("kprobe/tcp_v4_connect")
    int BPF_KPROBE(tcp_v4_connect, struct sock *sk)
    {
      /**
       * function body
      */   
    }

    SEC("kprobe/tcp_v6_connect")
    int BPF_KPROBE(tcp_v6_connect, struct sock *sk)
    {
      /**
       * function body
      */   
    }

    SEC("kprobe/tcp_rcv_state_process")
    int BPF_KPROBE(tcp_rcv_state_process, struct sock *sk)
    {
      /**
       * function body
      */   
    }
    ```
  - tcprtt  
  tcprtt为监测tcp连接往返时间的工具，其挂载点为fentry和kprobe中tcp连接建立相关的挂载点。
    ```c
    SEC("fentry/tcp_rcv_established")
    int BPF_PROG(tcp_rcv, struct sock *sk)
    {
      /**
       * function body
      */   
    }
    ```

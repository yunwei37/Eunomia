# 实际教学

由于eBPF是由事件触发的函数，所以在使用`libbpf-bootstrap`进行编程时，
首先需要找到内核代码运行时的执行点，然后在执行点书写处理函数。在完成内核态代码完成后，按照`*.skel.h`
的命名规则，撰写用户态代码，完成所有工作。 

本部分将就本项目书写的几个追踪模块进行讲解。
## process追踪模块

进程的追踪模块本项目主要设置了两个`tracepoint`挂载点。
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

## syscall追踪模块

对于系统调用的追踪模块设置了一个`tracepoint`挂载点。
挂载点形式为
```c
        SEC("tracepoint/raw_syscalls/sys_enter")
        int sys_enter(struct trace_event_raw_sys_enter *args)
        {
    
        }
```
当有syscall发生时，其经过`sys_enter`执行点时我们的函数将会被调用，将相关信息存入map后供用户态读取。

## file追踪模块
        对于文件系统，我们设置了两个`kprobe`挂载点
第一个挂载点形式为
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

## ipc追踪模块
        对于进程间通信的追踪，我们使用了Linux LSM模块的钩子，其形式为
```c
        SEC("lsm/ipc_permission")
        int BPF_PROG(ipc_permission, struct kern_ipc_perm *ipcp, short flag)
        {
    
        }
```
进程间发生了通信需要检查各自的权限时便会执行此函数

## tcp追踪模块

```c
SEC("kprobe/tcp_v6_connect")
int BPF_KPROBE(tcp_v6_connect, struct sock *sk) {
  return enter_tcp_connect(ctx, sk);
}

SEC("kretprobe/tcp_v6_connect")
int BPF_KRETPROBE(tcp_v6_connect_ret, int ret) {
  return exit_tcp_connect(ctx, ret, 6);
}
```
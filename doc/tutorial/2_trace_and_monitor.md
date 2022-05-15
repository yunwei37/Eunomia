## 实际教学
&ensp;&ensp;&ensp;&ensp;从之前的内容我们也可以发现，BPF编程的关键是找到正确的内核挂载点，本部分将就本项目书写的几个部分进行讲解
如何针对不同的模块书写代码。
### process追踪模块
&ensp;&ensp;&ensp;&ensp;进程的追踪模块本项目主要设置了两个`tracepoint`挂载点，分别是
`SEC("tp/sched/sched_process_exec")`和`SEC("tp/sched/sched_process_exit")`
当该进程被执行时，第一个挂载点下的执行函数将会被调用，记录信息后存入环形存储器。同样的，当有进程退出时，第二个
挂载点下的执行函数也将被调用并执行相应的操作。

## syscall追踪模块
&ensp;&ensp;&ensp;&ensp;对于系统调用的追踪模块设置了一个`tracepoint`挂载点，为`SEC("tracepoint/raw_syscalls/sys_enter")`。
当有syscall发生时，我们书写的`int sys_enter(struct trace_event_raw_sys_enter *args){}`将会被调用，
将相关信息存入map后供用户态读取。

## file追踪模块
&ensp;&ensp;&ensp;&ensp;对于文件系统，我们设置了两个`kprobe`挂载点，分别是`SEC("kprobe/vfs_read")`和`SEC("kprobe/vfs_write")`。
当发生了文件系统的读或写时，对应的函数会被调用并记录信息。

## ipc追踪模块
&ensp;&ensp;&ensp;&ensp;对于进程间通信的追踪，我们使用了Linux自带的LSM模块的钩子，将函数挂载在`SEC("lsm/ipc_permission")`下
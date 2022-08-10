## Eunomia - Funclatency: 使用基于 eBPF 的云原生监控工具监控内核函数耗时

### 背景

快速便捷地掌握某个内核函数在系统中执行的耗时可以帮助开发者们更好地优化程序，写出更
高效的代码。在过去这个事情是一个繁琐的事情，用户往往需要自行添加代码进行打点计时，
而 `Funclatency` 工具通过ebpf技术，快速地实现了对函数耗时的追踪。

### 实现原理

`Funclatency` 定义了 kprobe 和 kretprobe， 分别作用于用户指定的函数被
执行前和退出后。当调用了函数时，`Funclatency`会进行打点计时操作，将pid和时
间点数据存入map中。在函数返回时，Funclatency会再进行一次打点计时操作，根据pid
从map中找到对应的进入时间，通过计算差值得到函数的执行时长。
```c
SEC("kprobe/dummy_kprobe")
int BPF_KPROBE(dummy_kprobe)
{
	u64 id = bpf_get_current_pid_tgid();
	u32 tgid = id >> 32;
	u32 pid = id;
	u64 nsec;

	if (filter_cg && !bpf_current_task_under_cgroup(&cgroup_map, 0))
		return 0;

	if (targ_tgid && targ_tgid != tgid)
		return 0;
	nsec = bpf_ktime_get_ns();
	bpf_map_update_elem(&starts, &pid, &nsec, BPF_ANY);

	return 0;
}

SEC("kretprobe/dummy_kretprobe")
int BPF_KRETPROBE(dummy_kretprobe)
{
	u64 *start;
	u64 nsec = bpf_ktime_get_ns();
	u64 id = bpf_get_current_pid_tgid();
	u32 pid = id;
	u64 slot, delta;

	if (filter_cg && !bpf_current_task_under_cgroup(&cgroup_map, 0))
		return 0;

	start = bpf_map_lookup_elem(&starts, &pid);
	if (!start)
		return 0;

	delta = nsec - *start;

	switch (units) {
	case USEC:
		delta /= 1000;
		break;
	case MSEC:
		delta /= 1000000;
		break;
	}

	slot = log2l(delta);
	if (slot >= MAX_SLOTS)
		slot = MAX_SLOTS - 1;
	__sync_fetch_and_add(&hist[slot], 1);

	return 0;
}

```

在得到数据后，`Funclatency` 会将在内核态对数据进行预处理，存入对应的直方图
map中。当用户中止程序时，`Funclatency` 会以直方图的形式将该函数的耗时分布
展现出来。

### Eunomia中使用方式


### 总结
`Funclatency` 使得开发者非常方便地实现对某个函数的耗时监控，从而更好地对程序
进行优化。
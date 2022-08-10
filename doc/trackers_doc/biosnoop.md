## Eunomia - Biosnoop: 使用基于 eBPF 的云原生监控工具监控块设备 I/O
### 背景

Biosnoop 会追踪并打印磁盘的 I/O 操作。

### 实现原理

Biosnoop 在 block_rq_insert, block_rq_issue 和 block_rq_complete 三个 tracepoint 下
挂载了处理函数。当磁盘I/O操作发生时，block_rq_insert 和 block_rq_issue 两个挂载点下的处理函数
会以该操作对应的 request queue 为键，其对应的操作类型和发生时间为值，插入哈希表中。
```c
SEC("tp_btf/block_rq_insert")
int BPF_PROG(block_rq_insert)
{
	if (LINUX_KERNEL_VERSION > KERNEL_VERSION(5, 10, 0))
		return trace_rq_start((void *)ctx[0], true);
	else
		return trace_rq_start((void *)ctx[1], true);
}

SEC("tp_btf/block_rq_issue")
int BPF_PROG(block_rq_issue)
{
	if (LINUX_KERNEL_VERSION > KERNEL_VERSION(5, 10, 0))
		return trace_rq_start((void *)ctx[0], false);
	else
		return trace_rq_start((void *)ctx[1], false);
}

```
在 fentry/blk_account_io_start 和 kprobe/blk_account_io_merge_bio 两个挂载点下，
Biosnoop 会统计进程信息，并以 request queue 为键，pid 和 command 为值，存入另一张哈希表中。
```c
static __always_inline
int trace_pid(struct request *rq)
{
	u64 id = bpf_get_current_pid_tgid();
	struct piddata piddata = {};

	piddata.pid = id;
	bpf_get_current_comm(&piddata.comm, sizeof(&piddata.comm));
	bpf_map_update_elem(&infobyreq, &rq, &piddata, 0);
	return 0;
}

SEC("fentry/blk_account_io_start")
int BPF_PROG(blk_account_io_start, struct request *rq)
{
	return trace_pid(rq);
}
```
在完成I/O操作后, block_rq_complete 挂载点下的处理函数会从两张哈希表中
根据 request queue 获得对应的信息并输出。
```c
SEC("tp_btf/block_rq_complete")
int BPF_PROG(block_rq_complete, struct request *rq, int error,
	     unsigned int nr_bytes)
{
	u64 slot, ts = bpf_ktime_get_ns();
	struct piddata *piddatap;
	struct event event = {};
	struct stage *stagep;
	s64 delta;

	stagep = bpf_map_lookup_elem(&start, &rq);
	if (!stagep)
		return 0;
	delta = (s64)(ts - stagep->issue);
	if (delta < 0)
		goto cleanup;
	piddatap = bpf_map_lookup_elem(&infobyreq, &rq);
	if (!piddatap) {
		event.comm[0] = '?';
	} else {
		__builtin_memcpy(&event.comm, piddatap->comm,
				sizeof(event.comm));
		event.pid = piddatap->pid;
	}
	event.delta = delta;
	if (targ_queued && BPF_CORE_READ(rq, q, elevator)) {
		if (!stagep->insert)
			event.qdelta = -1; /* missed or don't insert entry */
		else
			event.qdelta = stagep->issue - stagep->insert;
	}
	event.ts = ts;
	event.sector = rq->__sector;
	event.len = rq->__data_len;
	event.cmd_flags = rq->cmd_flags;
	event.dev = stagep->dev;
	bpf_perf_event_output(ctx, &events, BPF_F_CURRENT_CPU, &event,
			sizeof(event));

cleanup:
	bpf_map_delete_elem(&start, &rq);
	bpf_map_delete_elem(&infobyreq, &rq);
	return 0;
}

```

### Eunomia中使用方式


### 总结
Biosnoop 通过将 tracepoint, kprobe 和 fentry 三类挂载点结合，实现了对磁盘I/O操作的统计。
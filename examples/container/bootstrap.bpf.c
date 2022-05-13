#include <vmlinux.h>
#include "bootstrap.h"
#include <asm-generic/errno.h>
#include <bpf/bpf_core_read.h>
#include <bpf/bpf_helpers.h>
#include <bpf/bpf_tracing.h>

#define MAX_ENTRIES 8 * 1024
#define MAX_SYSCALLS 1024
#define MAX_COMM_LEN 64

char LICENSE[] SEC("license") = "Dual BSD/GPL";

struct
{
    __uint(type, BPF_MAP_TYPE_HASH);
    __uint(max_entries, MAX_ENTRIES);
    __type(key, u32);
    __type(value, struct event);
} processes SEC(".maps");

struct
{
    __uint(type, BPF_MAP_TYPE_RINGBUF);
    __uint(max_entries, 1 << 24);
} events SEC(".maps");


// SEC("tracepoint/syscalls/sys_enter_fork")
// int enter_fork() {

// }
// SEC("uprobe/do_containerize")
// int BPF_KPROBE(do_containerize, int *ret_p)
// {
//     pid_t pid;
    
//     pid = bpf_get_current_pid_tgid() >> 32;
//     struct event *event = bpf_ringbuf_reserve(&events, sizeof(struct event), 0);
//     if(!event) {
//         return 0;
//     }
//     event->hpid = 10;
//     event->container_id = 10;
//     event->cpid = 10;
//     bpf_ringbuf_submit(event, 0);
//     return 0;
// }

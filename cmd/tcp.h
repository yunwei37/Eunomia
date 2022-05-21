#ifndef TCP_CMD_H
#define TCP_CMD_H

#include "libbpf_print.h"
#include "tracker.h"

extern "C" {
#include "tcp/tcp_tracker.h"
}

struct tcp_tracker : public tracker {
    struct tcp_env env = {0};
    tcp_tracker() {
        tcp_tracker({0});
    }
    tcp_tracker(tcp_env env) {
        this->env = env;
        exiting = false;
        env.exiting = &exiting;
        env.collector = tcp_count_collector;
    }

    void start_tracker(void) {
        env.exiting = &exiting;
	    env.collector = tcp_count_collector;
	    return start_tcp_tracker(handle_event, libbpf_print_fn, env)
    }

    static void handle_event(void *ctx, int cpu, void *data, __u32 data_sz) {
        const struct tcp_event *tcp_event = data;
        char src[INET6_ADDRSTRLEN];
        char dst[INET6_ADDRSTRLEN];
        union {
            struct in_addr  x4;
            struct in6_addr x6;
        } s, d;
        static __u64 start_ts;

        if (tcp_event->af == AF_INET) {
            s.x4.s_addr = tcp_event->saddr_v4;
            d.x4.s_addr = tcp_event->daddr_v4;
        } else if (tcp_event->af == AF_INET6) {
            memcpy(&s.x6.s6_addr, tcp_event->saddr_v6, sizeof(s.x6.s6_addr));
            memcpy(&d.x6.s6_addr, tcp_event->daddr_v6, sizeof(d.x6.s6_addr));
        } else {
            warn("broken tcp_event: tcp_event->af=%d", tcp_event->af);
            return;
        }

        if (env.print_timestamp) {
            if (start_ts == 0)
                start_ts = tcp_event->ts_us;
            printf("%-9.3f", (tcp_event->ts_us - start_ts) / 1000000.0);
        }

        if (env.print_uid)
            printf("%-6d", tcp_event->uid);

        printf("%-6d %-12.12s %-2d %-16s %-16s %-4d\n",
            tcp_event->pid, tcp_event->task,
            tcp_event->af == AF_INET ? 4 : 6,
            inet_ntop(tcp_event->af, &s, src, sizeof(src)),
            inet_ntop(tcp_event->af, &d, dst, sizeof(dst)),
            ntohs(tcp_event->dport));
    }
}

#endif

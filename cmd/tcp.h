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
        // env.exiting = &exiting;
    }

    void start_tracker() {
        LIBBPF_OPTS(bpf_object_open_opts, open_opts);
        struct tcp_bpf *obj;
        start_tcp_tracker(print_count, print_events, libbpf_print_fn, open_opts, obj, env);
    }

    static void print_count(int map_fd_ipv4, int map_fd_ipv6) {
        static const char *header_fmt = "\n%-25s %-25s %-20s %-10s\n";

        while (!exiting)
            pause();

        printf(header_fmt, "LADDR", "RADDR", "RPORT", "CONNECTS");
        print_count_ipv4(map_fd_ipv4);
        print_count_ipv6(map_fd_ipv6);
    }

    static void print_count_ipv4(int map_fd) {
        static struct ipv4_flow_key keys[MAX_ENTRIES];
        __u32 value_size = sizeof(__u64);
        __u32 key_size = sizeof(keys[0]);
        static struct ipv4_flow_key zero;
        static __u64 counts[MAX_ENTRIES];
        char s[INET_ADDRSTRLEN];
        char d[INET_ADDRSTRLEN];
        __u32 i, n = MAX_ENTRIES;
        struct in_addr src;
        struct in_addr dst;

        for (i = 0; i < n; i++) {
            src.s_addr = keys[i].saddr;
            dst.s_addr = keys[i].daddr;

            printf("%-25s %-25s %-20d %-10llu\n",
                inet_ntop(AF_INET, &src, s, sizeof(s)),
                inet_ntop(AF_INET, &dst, d, sizeof(d)),
                ntohs(keys[i].dport), counts[i]);
        }
    }

    static void print_count_ipv6(int map_fd) {
        static struct ipv6_flow_key keys[MAX_ENTRIES];
        __u32 value_size = sizeof(__u64);
        __u32 key_size = sizeof(keys[0]);
        static struct ipv6_flow_key zero;
        static __u64 counts[MAX_ENTRIES];
        char s[INET6_ADDRSTRLEN];
        char d[INET6_ADDRSTRLEN];
        __u32 i, n = MAX_ENTRIES;
        struct in6_addr src;
        struct in6_addr dst;

        for (i = 0; i < n; i++) {
            memcpy(src.s6_addr, keys[i].saddr, sizeof(src.s6_addr));
            memcpy(dst.s6_addr, keys[i].daddr, sizeof(src.s6_addr));

            printf("%-25s %-25s %-20d %-10llu\n",
                inet_ntop(AF_INET6, &src, s, sizeof(s)),
                inet_ntop(AF_INET6, &dst, d, sizeof(d)),
                ntohs(keys[i].dport), counts[i]);
        }
    }

    static void print_events_header() {
        if (env.print_timestamp)
            printf("%-9s", "TIME(s)");
        if (env.print_uid)
            printf("%-6s", "UID");
        printf("%-6s %-12s %-2s %-16s %-16s %-4s\n",
            "PID", "COMM", "IP", "SADDR", "DADDR", "DPORT");
    }

    static void handle_event(void *ctx, int cpu, void *data, __u32 data_sz) {
        const struct event *event = data;
        char src[INET6_ADDRSTRLEN];
        char dst[INET6_ADDRSTRLEN];
        union {
            struct in_addr  x4;
            struct in6_addr x6;
        } s, d;
        static __u64 start_ts;

        if (event->af == AF_INET) {
            s.x4.s_addr = event->saddr_v4;
            d.x4.s_addr = event->daddr_v4;
        } else if (event->af == AF_INET6) {
            memcpy(&s.x6.s6_addr, event->saddr_v6, sizeof(s.x6.s6_addr));
            memcpy(&d.x6.s6_addr, event->daddr_v6, sizeof(d.x6.s6_addr));
        } else {
            warn("broken event: event->af=%d", event->af);
            return;
        }

        if (env.print_timestamp) {
            if (start_ts == 0)
                start_ts = event->ts_us;
            printf("%-9.3f", (event->ts_us - start_ts) / 1000000.0);
        }

        if (env.print_uid)
            printf("%-6d", event->uid);

        printf("%-6d %-12.12s %-2d %-16s %-16s %-4d\n",
            event->pid, event->task,
            event->af == AF_INET ? 4 : 6,
            inet_ntop(event->af, &s, src, sizeof(src)),
            inet_ntop(event->af, &d, dst, sizeof(dst)),
            ntohs(event->dport));
    }

    static void handle_lost_events(void *ctx, int cpu, __u64 lost_cnt) {
        warn("Lost %llu events on CPU #%d!\n", lost_cnt, cpu);
    }

    static void print_events(int perf_map_fd) {
        struct perf_buffer *pb;
        int err;

        pb = perf_buffer__new(perf_map_fd, 128,
                    handle_event, handle_lost_events, NULL, NULL);
        if (!pb) {
            err = -errno;
            warn("failed to open perf buffer: %d\n", err);
            goto cleanup;
        }

        print_events_header();
        while (!exiting) {
            err = perf_buffer__poll(pb, 100);
            if (err < 0 && err != -EINTR) {
                warn("error polling perf buffer: %s\n", strerror(-err));
                goto cleanup;
            }
            /* reset err to return 0 if exiting */
            err = 0;
        }

    cleanup:
        perf_buffer__free(pb);
    }

}

#endif

#ifndef TCP_TRACKER_H
#define TCP_TRACKER_H

#include <sys/resource.h>
#include <arpa/inet.h>
#include <argp.h>
#include <signal.h>
#include <limits.h>
#include <unistd.h>
#include <time.h>
#include <bpf/bpf.h>

struct tcp_env {
	bool verbose;
	bool count;
	bool print_timestamp;
	bool print_uid;
	pid_t pid;
	uid_t uid;
	int nports;
	int ports[MAX_PORTS];

    bool is_csv;
};

static int start_tcp_tracker(void (*print_count)(int,int), void (*print_events)(int), libbpf_print_fn_t libbpf_print_fn, struct bpf_object_open_opts open_opts, struct tcp_bpf *obj,struct tcp_env env)
{
	libbpf_set_strict_mode(LIBBPF_STRICT_ALL);
	/* Set up libbpf errors and debug info callback */
	libbpf_set_print(libbpf_print_fn);

    /* Load and verify BPF application */
    obj = tcp_bpf__open_opts(&open_opts);
	if (!obj) {
		warn("failed to open BPF object\n");
		return 1;
	}

    int i, err;
    if (env.count)
		obj->rodata->do_count = true;
	if (env.pid)
		obj->rodata->filter_pid = env.pid;
	if (env.uid != (uid_t) -1)
		obj->rodata->filter_uid = env.uid;
	if (env.nports > 0) {
		obj->rodata->filter_ports_len = env.nports;
		for (i = 0; i < env.nports; i++) {
			obj->rodata->filter_ports[i] = htons(env.ports[i]);
		}
	}

    /* Load & verify BPF programs */
    err = tcp_bpf__load(obj);
	if (err) {
		warn("failed to load BPF object: %d\n", err);
		goto cleanup;
	}

    /* Attach tracepoints */
	err = tcp_bpf__attach(obj);
	if (err) {
		warn("failed to attach BPF programs: %s\n", strerror(-err));
		goto cleanup;
	}

	if (env.count) {
		print_count(bpf_map__fd(obj->maps.ipv4_count),
			    bpf_map__fd(obj->maps.ipv6_count));
	} else {
		print_events(bpf_map__fd(obj->maps.events));
	}

cleanup:
	tcp_bpf__destroy(obj);

	return err != 0;
}

#endif /* TCP_TRACKER_H */

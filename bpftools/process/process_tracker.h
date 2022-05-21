// SPDX-License-Identifier: GPL-2.0 OR BSD-3-Clause
#ifndef PROCESS_TRACKER_H
#define PROCESS_TRACKER_H

#include <argp.h>
#include <signal.h>
#include <stdio.h>
#include <time.h>
#include <sys/resource.h>
#include <bpf/libbpf.h>
#include "process.h"
#include "process.skel.h"
#include "output.h"

struct process_env
{
	bool verbose;
	bool is_csv;
	pid_t target_pid;
	long min_duration_ms;
	volatile bool *exiting;
};

static int start_process_tracker(ring_buffer_sample_fn handle_event, libbpf_print_fn_t libbpf_print_fn, 
	struct process_env env, struct process_bpf *skel)
{
	struct ring_buffer *rb = NULL;
	int err;

	if (!env.exiting) {
		fprintf(stderr, "env.exiting is not set.\n");
		return -1;
	}
	libbpf_set_strict_mode(LIBBPF_STRICT_ALL);
	/* Set up libbpf errors and debug info callback */
	libbpf_set_print(libbpf_print_fn);

	/* Load and verify BPF application */
	skel = process_bpf__open();
	if (!skel)
	{
		fprintf(stderr, "Failed to open and load BPF skeleton\n");
		return 1;
	}

	/* Parameterize BPF code with minimum duration parameter */
	skel->rodata->min_duration_ns = env.min_duration_ms * 1000000ULL;
	skel->rodata->target_pid = env.target_pid;

	/* Load & verify BPF programs */
	err = process_bpf__load(skel);
	if (err)
	{
		fprintf(stderr, "Failed to load and verify BPF skeleton\n");
		goto cleanup;
	}

	/* Attach tracepoints */
	err = process_bpf__attach(skel);
	if (err)
	{
		fprintf(stderr, "Failed to attach BPF skeleton\n");
		goto cleanup;
	}

	/* Set up ring buffer polling */
	rb = ring_buffer__new(bpf_map__fd(skel->maps.rb), handle_event, NULL, NULL);
	if (!rb)
	{
		err = -1;
		fprintf(stderr, "Failed to create ring buffer\n");
		goto cleanup;
	}
	while (!(*env.exiting))
	{
		err = ring_buffer__poll(rb, 100 /* timeout, ms */);
		/* Ctrl-C will cause -EINTR */
		if (err == -EINTR)
		{
			err = 0;
			break;
		}
		if (err < 0)
		{
			printf("Error polling perf buffer: %d\n", err);
			break;
		}
	}

cleanup:
	/* Clean up */
	ring_buffer__free(rb);
	process_bpf__destroy(skel);

	return err < 0 ? -err : 0;
}

#endif
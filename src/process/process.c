// SPDX-License-Identifier: (LGPL-2.1 OR BSD-2-Clause)
/* Copyright (c) 2020 Facebook */
#include <argp.h>
#include <signal.h>
#include <stdio.h>
#include <time.h>
#include <sys/resource.h>
#include <bpf/libbpf.h>
#include "process.h"
#include "process.skel.h"
#include "output.h"

static struct env
{
	bool verbose;
	bool is_csv;
	long min_duration_ms;
} env;

const char *argp_program_version = "process 1.0";
const char *argp_program_bug_address = "<1067852565@qq.com>";
const char argp_program_doc[] =
	"eBPF process tracing application.\n"
	"\n"
	"It traces process start and exits and shows associated \n"
	"information (filename, process duration, PID and PPID, etc).\n"
	"\n"
	"USAGE: ./process [-d <min-duration-ms>] [-v]\n";

static const struct argp_option opts[] = {
	{"verbose", 'v', NULL, 0, "Verbose debug output"},
	{"verbose", 'C', NULL, 0, "Output in the CSV format"},
	{"duration", 'd', "DURATION-MS", 0, "Minimum process duration (ms) to report"},
	{},
};

static error_t parse_arg(int key, char *arg, struct argp_state *state)
{
	switch (key)
	{
	case 'v':
		env.verbose = true;
		break;
	case 'd':
		errno = 0;
		env.min_duration_ms = strtol(arg, NULL, 10);
		if (errno || env.min_duration_ms <= 0)
		{
			fprintf(stderr, "Invalid duration: %s\n", arg);
			argp_usage(state);
		}
		break;
	case ARGP_KEY_ARG:
		argp_usage(state);
		break;
	default:
		return ARGP_ERR_UNKNOWN;
	}
	return 0;
}

static const struct argp argp = {
	.options = opts,
	.parser = parse_arg,
	.doc = argp_program_doc,
};

static int libbpf_print_fn(enum libbpf_print_level level, const char *format, va_list args)
{
	if (level == LIBBPF_DEBUG && !env.verbose)
		return 0;
	return vfprintf(stderr, format, args);
}

static volatile bool exiting = false;

static void sig_handler(int sig)
{
	exiting = true;
}

static void print_table_data(const struct event *e)
{
	print_basic_info(e, false);

	if (e->exit_event)
	{
		printf("%-5s %-16s [%u]",
			   "EXIT", e->comm, e->exit_code);
		if (e->duration_ns)
			printf(" (%llums)", e->duration_ns / 1000000);
		printf("\n");
	}
	else
	{
		printf("%-5s %-16s %s\n",
			   "EXEC", e->comm, e->filename);
	}
}

static void
print_header(void)
{
	print_table_header(headers, env.is_csv);
}

static void
print_csv_data(const struct event *e)
{
	print_basic_info(e, true);

	if (e->exit_event)
	{
		printf("%s,%s,%u,",
			   "EXIT", e->comm, e->exit_code);
		if (e->duration_ns)
			printf("%llu", e->duration_ns / 1000000);
		printf("\n");
	}
	else
	{
		printf("%s,%s,%s,\n",
			   "EXEC", e->comm, e->filename);
	}
}

static int handle_event(void *ctx, void *data, size_t data_sz)
{
	const struct event *e = data;
	if (env.is_csv)
		print_csv_data(e);
	else
		print_table_data(e);
	return 0;
}

int main(int argc, char **argv)
{
	struct ring_buffer *rb = NULL;
	struct process_bpf *skel;
	int err;

	/* Parse command line arguments */
	err = argp_parse(&argp, argc, argv, 0, NULL, NULL);
	if (err)
		return err;

	libbpf_set_strict_mode(LIBBPF_STRICT_ALL);
	/* Set up libbpf errors and debug info callback */
	libbpf_set_print(libbpf_print_fn);

	/* Cleaner handling of Ctrl-C */
	signal(SIGINT, sig_handler);
	signal(SIGTERM, sig_handler);

	/* Load and verify BPF application */
	skel = process_bpf__open();
	if (!skel)
	{
		fprintf(stderr, "Failed to open and load BPF skeleton\n");
		return 1;
	}

	/* Parameterize BPF code with minimum duration parameter */
	skel->rodata->min_duration_ns = env.min_duration_ms * 1000000ULL;

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
	print_header();
	while (!exiting)
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

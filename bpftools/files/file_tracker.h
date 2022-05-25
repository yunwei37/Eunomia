#ifndef FILE_TRACKER_H
#define FILE_TRACKER_H

#include <argp.h>
#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <bpf/libbpf.h>
#include <bpf/bpf.h>
#include <stdbool.h>
#include "files.h"
#include "files.skel.h"

#define warn(...) fprintf(stderr, __VA_ARGS__)
#define OUTPUT_ROWS_LIMIT 10240

struct files_env {
    volatile int *exiting;
    pid_t target_pid;
    bool clear_screen;
    bool regular_file_only;
    int output_rows;
    int sort_by;
    int interval;
    int count;
    bool verbose;
    void *ctx;
}; 

struct file_event {
    int rows;
    struct file_stat *values;
};

static int print_stat(struct files_bpf *obj, struct files_env env, ring_buffer_sample_fn handle_event)
{
	struct file_id key, *prev_key = NULL;
	static struct file_stat values[OUTPUT_ROWS_LIMIT];
	int  err = 0, rows = 0;
	int fd = bpf_map__fd(obj->maps.entries);

	while (1) {
		err = bpf_map_get_next_key(fd, prev_key, &key);
		if (err) {
			if (errno == ENOENT) {
				err = 0;
				break;
			}
			warn("bpf_map_get_next_key failed: %s\n", strerror(errno));
			return err;
		}
		err = bpf_map_lookup_elem(fd, &key, &values[rows++]);
		if (err) {
			warn("bpf_map_lookup_elem failed: %s\n", strerror(errno));
			return err;
		}
		prev_key = &key;
	}
    struct file_event e = {
        .rows = rows,
        .values = values,
    };
    handle_event(env.ctx, &e, sizeof(e));
    
	prev_key = NULL;

	while (1) {
		err = bpf_map_get_next_key(fd, prev_key, &key);
		if (err) {
			if (errno == ENOENT) {
				err = 0;
				break;
			}
			warn("bpf_map_get_next_key failed: %s\n", strerror(errno));
			return err;
		}
		err = bpf_map_delete_elem(fd, &key);
		if (err) {
			warn("bpf_map_delete_elem failed: %s\n", strerror(errno));
			return err;
		}
		prev_key = &key;
	}
	return err;
}


static int start_file_tracker(ring_buffer_sample_fn handle_event, libbpf_print_fn_t libbpf_print_fn, struct files_env env)
{
	LIBBPF_OPTS(bpf_object_open_opts, open_opts);
	struct files_bpf *obj;
	int err;

    if (!env.exiting)
    {
        fprintf(stderr, "env.exiting is not set.\n");
        return -1;
    }
	libbpf_set_strict_mode(LIBBPF_STRICT_ALL);
	libbpf_set_print(libbpf_print_fn);

	obj = files_bpf__open_opts(&open_opts);
	if (!obj) {
		warn("failed to open BPF object\n");
		return 1;
	}

	obj->rodata->target_pid = env.target_pid;
	obj->rodata->regular_file_only = env.regular_file_only;

	err = files_bpf__load(obj);
	if (err) {
		warn("failed to load BPF object: %d\n", err);
		goto cleanup;
	}

	err = files_bpf__attach(obj);
	if (err) {
		warn("failed to attach BPF programs: %d\n", err);
		goto cleanup;
	}

	while (1) {
		sleep(env.interval);

		if (env.clear_screen) {
			err = system("clear");
			if (err)
				goto cleanup;
		}

		err = print_stat(obj, env, handle_event);
		if (err)
			goto cleanup;

		env.count--;
		if (*env.exiting || !env.count)
			goto cleanup;
	}

cleanup:
	files_bpf__destroy(obj);

	return err != 0;
}

#endif

#ifndef UPDATE_TRACKER_H
#define UPDATE_TRACKER_H

extern "C"
{
// SPDX-License-Identifier: (LGPL-2.1 OR BSD-2-Clause)
/* Copyright (c) 2020 Facebook */
#include <argp.h>
#include <signal.h>
#include <stdio.h>
#include <time.h>
#include <sys/resource.h>
#include <bpf/libbpf.h>
#include "update.h"
#include "update.skel.h"
#include "base64.h"
}
#include "hot_update.h"

static const char argp_program_doc[] =
    "BPF update demo application.\n"
    "\n"
    "It traces process start and exits and shows associated \n"
    "information (filename, process duration, PID and PPID, etc).\n"
    "\n"
    "USAGE: ./update [-d <min-duration-ms>] [-v]\n";

// need to be freed after used
const unsigned char *base64_decode_buffer = NULL;

static volatile bool exiting = false;

static void sig_handler(int sig)
{
  exiting = true;
}

static int handle_event(void *ctx, void *data, size_t data_sz)
{
  const struct event *e = (const struct event *)data;
  struct tm *tm;
  char ts[32];
  time_t t;

  time(&t);
  tm = localtime(&t);
  strftime(ts, sizeof(ts), "%H:%M:%S", tm);

  if (e->exit_event)
  {
    printf("%-8s %-5s %-16s %-7d %-7d [%u]", ts, "EXIT", e->comm, e->pid, e->ppid, e->exit_code);
    if (e->duration_ns)
      printf(" (%llums)", e->duration_ns / 1000000);
    printf("\n");
  }
  else
  {
    printf("%-8s %-5s %-16s %-7d %-7d %s\n", ts, "EXEC", e->comm, e->pid, e->ppid, e->filename);
  }
  return 0;
}

static inline int create_skeleton_from_json(struct update_bpf *obj, const struct ebpf_update_data &ebpf_data)
{
  struct bpf_object_skeleton *s;
  size_t out_len;

  s = (struct bpf_object_skeleton *)calloc(1, sizeof(*s));
  if (!s)
    return -1;
  obj->skeleton = s;

  s->sz = sizeof(*s);
  s->name = ebpf_data.name.c_str();
  s->obj = &obj->obj;

  /* maps */
  s->map_cnt = ebpf_data.maps_name.size();
  s->map_skel_sz = sizeof(*s->maps);
  s->maps = (struct bpf_map_skeleton *)calloc(s->map_cnt, s->map_skel_sz);
  if (!s->maps)
    goto err;

  for (int i = 0; i < s->map_cnt; i++)
  {
    s->maps[i].name = ebpf_data.maps_name[i].c_str();
    s->maps[i].map = &obj->maps.rb;
  }

  /* programs */
  s->prog_cnt = ebpf_data.progs_name.size();
  s->prog_skel_sz = sizeof(*s->progs);
  s->progs = (struct bpf_prog_skeleton *)calloc(s->prog_cnt, s->prog_skel_sz);
  if (!s->progs)
    goto err;
  for (int i = 0; i < s->prog_cnt; i++)
  {
    s->progs[i].name = ebpf_data.progs_name[i].c_str();
    s->progs[i].prog = &obj->progs.handle_exec;
    s->progs[i].link = &obj->links.handle_exec;
  }

  s->data_sz = ebpf_data.data_sz;
  base64_decode_buffer = base64_decode((const unsigned char *)ebpf_data.data.c_str(), ebpf_data.data.size(), &out_len);
  // s->data = ebpf_raw_data;
  s->data = (void *)base64_decode_buffer;

  return 0;
err:
  bpf_object__destroy_skeleton(s);
  return -1;
}

static inline struct update_bpf *update_bpf__open_from_json(const struct ebpf_update_data &ebpf_data)
{
  struct update_bpf *obj;

  obj = (struct update_bpf *)calloc(1, sizeof(*obj));
  if (!obj)
    return NULL;
  if (create_skeleton_from_json(obj, ebpf_data))
    goto err;
  if (bpf_object__open_skeleton(obj->skeleton, NULL))
    goto err;

  return obj;
err:
  update_bpf__destroy(obj);
  return NULL;
}

static int start_updatable(int argc, char **argv)
{
  struct ring_buffer *rb = NULL;
  struct update_bpf *skel;
  int err;

  if (argc != 2)
  {
    printf("invalid arg count %d\n", argc);
    return 1;
  }
  std::string json_str = argv[1];
  printf("%s", argv[1]);

  libbpf_set_strict_mode(LIBBPF_STRICT_ALL);

  /* Cleaner handling of Ctrl-C */
  signal(SIGINT, sig_handler);
  signal(SIGTERM, sig_handler);

  /* Load and verify BPF application */
  struct ebpf_update_data ebpf_data;
  ebpf_data.from_json_str(json_str);
  skel = update_bpf__open_from_json(ebpf_data);
  if (!skel)
  {
    fprintf(stderr, "Failed to open and load BPF skeleton\n");
    return 1;
  }

  /* Load & verify BPF programs */
  err = update_bpf__load(skel);
  if (err)
  {
    fprintf(stderr, "Failed to load and verify BPF skeleton\n");
    goto cleanup;
  }

  /* Attach tracepoints */
  err = update_bpf__attach(skel);
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

  /* Process events */
  printf("%-8s %-5s %-16s %-7s %-7s %s\n", "TIME", "EVENT", "COMM", "PID", "PPID", "FILENAME/EXIT CODE");
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
  update_bpf__destroy(skel);

  return err < 0 ? -err : 0;
}

#endif

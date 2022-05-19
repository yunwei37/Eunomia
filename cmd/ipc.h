#ifndef IPC_CMD_H
#define IPC_CMD_H

#include "libbpf_print.h"
#include "tracker.h"

extern "C" {
#include "ipc/ipc_tracker.h"
}

struct ipc_tracker : public tracker {
  struct ipc_env env = {0};
  ipc_tracker() {
    env = {0};
    exiting = false;
    env.exiting = &exiting;
  }

  void start_tracker() {
    start_ipc_tracker(handle_event, libbpf_print_fn, env);
  }

  static int handle_event(void *ctx, void *data, size_t data_sz) {
    const struct ipc_event *e = (const struct ipc_event *)data;
    struct tm *tm;
    char ts[32];
    time_t t;

    time(&t);
    tm = localtime(&t);
    strftime(ts, sizeof(ts), "%H:%M:%S", tm);

    printf("%-8s %u %u %u [%u] %u\n", ts, e->pid, e->uid, e->gid, e->cuid,
           e->cgid);

    return 0;
  }
};

#endif
#ifndef CONTAINER_CMD_H
#define CONTAINER_CMD_H

#include <iostream>

#include "libbpf_print.h"

extern "C" {
#include "container/container_tracker.h"
}

struct container_tracker
{
    volatile bool exiting;
    
    struct container_env env = {0};
    
    container_tracker() {
        env = {0};
        exiting = false;
        env.exiting = &exiting;
    }

    void start_container() {
        start_container_tracker(handle_event, libbpf_print_fn, env);
    }

    static int handle_event(void *ctx, void *data, size_t data_sz)
    {
        const struct container_event *e = (const struct container_event *)data;
        printf("%-10u %-15u %lu \n", e->pid, e->ppid, e->container_id);
        return 0;
    }
};


#endif
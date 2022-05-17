#ifndef EBPF_H
#define EBPF_H

extern "C" {
#include "process/process_tracker.h"
}


static int libbpf_print_fn(enum libbpf_print_level level, const char *format, va_list args)
{
	return 0 && vfprintf(stderr, format, args);
}

static int handle_event(void *ctx, void *data, size_t data_sz)
{
	const struct common_event* e = (const struct common_event*)data;
	print_basic_info(e, false);
	return 0;
}

// thread example
#include <iostream>       // std::cout
#include <thread>         // std::thread

volatile bool isexit = false;

int start_process_thread() {
	struct process_env env = {0};
	env.exiting = &isexit;
	std::cout << "start_process_thread\n";
    return start_process_tracker(handle_event, libbpf_print_fn, env);
}


#endif
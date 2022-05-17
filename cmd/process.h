#ifndef PROCESS_CMD_H
#define PROCESS_CMD_H

#include <iostream>
#include <thread>
#include <mutex>
#include "libbpf_print.h"

extern "C"
{
#include "process/process_tracker.h"
}

struct process_tracker
{
	volatile bool exiting;
	std::mutex mutex;
	struct process_env env = {0};
	process_tracker()
	{
		env = {0};
		exiting = false;
		env.exiting = &exiting;
	}
	process_tracker(pid_t target_pid,
					long min_duration_ms)
	{
		exiting = false;
		env = {0};
		env.exiting = &exiting;
		env.target_pid = target_pid;
		env.min_duration_ms = min_duration_ms;
	}
	void start_process()
	{
		start_process_tracker(handle_event, libbpf_print_fn, env);
	}
	static int handle_event(void *ctx, void *data, size_t data_sz)
	{
		const struct common_event *e = (const struct common_event *)data;
		print_basic_info(e, false);
		return 0;
	}
};

#endif
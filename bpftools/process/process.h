// SPDX-License-Identifier: GPL-2.0 OR BSD-3-Clause
#ifndef PROCESS_H
#define PROCESS_H

#include "event.h"

struct process_event
{
	struct common_event common;

	unsigned exit_code;
	unsigned long long duration_ns;
	char comm[TASK_COMM_LEN];
	char filename[MAX_FILENAME_LEN];
	bool exit_event;
};

#endif /* PROCESS_H */
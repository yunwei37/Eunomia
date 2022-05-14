#ifndef PROCESS_H
#define PROCESS_H

#include "event.h"

struct event {
    struct common_event common;

	unsigned exit_code;
	unsigned long long duration_ns;
	char comm[TASK_COMM_LEN];
	char filename[MAX_FILENAME_LEN];
	bool exit_event;
};

static const char* headers[] = {
	"stat", "comm", "filename/exitcode", "duration",
	NULL
};

#endif
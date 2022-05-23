#ifndef __CONTAINER_H
#define __CONTAINER_H

#include <process/process.h>

struct container_event {
	struct process_event process;
    // container this process belongs to
	unsigned long container_id; // container_id
};

#endif
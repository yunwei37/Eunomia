#ifndef __CONTAINER_H
#define __CONTAINER_H

#include "../process/process.h"

struct container_event {
	// int pid;
	// int ppid;
	struct process_event process;
	unsigned long container_id;
	std::string container_name;
};

#endif
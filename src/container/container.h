#ifndef __BOOTSTRAP_H
#define __BOOTSTRAP_H

#define TASK_COMM_LEN 16
#define MAX_FILENAME_LEN 127
#define CONTAINER_ID_LEN 127

struct container_event {
    // container this process belongs to
	unsigned long container_id; // container_id
	// pid in host
	int pid;   
	// parent pid of pid
	int ppid;
};

#endif /* __BOOTSTRAP_H */